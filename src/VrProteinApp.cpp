/***********************************************************************
 VrProteinApp
 Copyright (c) 2014 Pablo Cruz

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ***********************************************************************/

#include "utils/backtrace.h"
#include <iostream>
//#include <GL/GLModels.h>
//#include <GL/GLMaterialTemplates.h>
//#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Button.h>
#include <GLMotif/Label.h>
#include <GLMotif/Menu.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetManager.h>
#include <Misc/File.h>
#include <Vrui/CoordinateManager.h>
#include <Vrui/ToolManager.h>
#include "VrProteinApp.h"
#include "DrawMolecule.h"
#include "HudWidget.h"
#include "Molecule.h"
#include "MoleculeDragger.h"
#include "PDBImporter.h"
#include "ExperimentControlTool.h"
#include "SimulationControlTool.h"
#include "utils/datetime.h"
#include "utils/string.h"

using std::unique_ptr;

using GLMotif::Button;
using GLMotif::DropdownBox;
using GLMotif::Label;
using GLMotif::Menu;
using GLMotif::PopupMenu;
using GLMotif::PopupWindow;
using GLMotif::RadioBox;
using GLMotif::RowColumn;
using GLMotif::TextField;
using GLMotif::ToggleButton;

using Misc::CallbackData;

namespace VrProtein {

VrProteinApp::VrProteinApp(int& argc, char**& argv) :
			Vrui::Application(argc, argv),
			selectedStyle(DrawStyle::Surf),
			selectedColorStyle(ColorStyle::CPK),
			selectedMoleculeIdx(0),
			isSimulating(true),
			isCalculatingForces(false) {

	/* load molecule data */
	drawMolecules.push_back(CreateMolecule("alanin/alanin.pdb"));
	drawMolecules.push_back(CreateMolecule("alanin/alanin.pdb"));

	/* Move them away */
	drawMolecules.at(0)->SetState(ONTransform::translateFromOriginTo(Point(-10, 0, 0)));
	drawMolecules.at(1)->SetState(ONTransform::translateFromOriginTo(Point(10, 0, 0)));

	/* Set the navigation transformation to show the entire scene: */
	centerDisplay();
	/* Set the navigational coordinate system unit: */
	Vrui::getCoordinateManager()->setUnit(Geometry::LinearUnit(Geometry::LinearUnit::ANGSTROM, 1));

	/* Create the program's user interface: */
	createMainMenu();
	Vrui::setMainMenu(mainMenuPopup);
	createSettingsDialog();
	createStatisticsDialog();

	/* Load widgets */
	overlapWidget = unique_ptr<HudWidget>(
			new HudWidget("OverlapWidget", Vrui::getWidgetManager(), "Overlap"));
	overlapWidget->setOptions(false, 0, 10, false, "%4.2f");

	distanceWidget = unique_ptr<HudWidget>(
			new HudWidget("DistanceWidget", Vrui::getWidgetManager(), "Mean dist."));
	distanceWidget->setOptions(false, 0, 5, false, "%4.2f");

	/* Set up custom tools: */
	SimulationControlTool::registerTool(*Vrui::getToolManager());
	ExperimentControlTool::registerTool(*Vrui::getToolManager());

	/* Tell Vrui to run in a continuous frame sequence: */
	Vrui::updateContinuously();

	/* Initialize the frame time calculator: */
	lastFrameTime = Vrui::getApplicationTime();

	std::cout << "All loaded. Starting main loop." << std::endl;
}

void VrProteinApp::display(GLContextData& contextData) const {
	// Draw domain box
	domainBox.glRenderAction(contextData);
	// Draw molecules
	for (const auto& m : drawMolecules) {
		m->glRenderAction(contextData);
	}

	for(const auto& it : bestDrawMolecules) {
		it.second->glRenderAction(contextData);
	}

	if (isSimulating && isCalculatingForces) {
		// Draw force arrow
		/*
		Scalar netForceMag = simResult.netForce.mag();
		if (netForceMag > 0.5f) {
			auto arrowColor = GLColor<GLfloat, 4>(0.3f, 0.9f, 0.3f); // green
			if (simResult.energy < 0)
				arrowColor = GLColor<GLfloat, 4>(0.9f, 0.0f, 0.3f); // red
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, arrowColor);

			glPushMatrix();
			{
				Vector rotAxis = Geometry::cross(Vrui::getUpDirection(), simResult.netForce);
				Scalar rotAngle = Math::acos(simResult.netForce/netForceMag * Vrui::getUpDirection());
				glMultMatrix(RotTransform(Rotation(rotAxis, rotAngle)));
				glDrawArrow(0.5f, 1.0f, 1.0f, Math::min(2 * netForceMag, 25.0), 6);
			}
			glPopMatrix();
		}
		*/
		// Draw torque arrow
		/*
		Scalar netTorqueMag = netTorque.mag();
		if (netTorqueMag > 0.5f) {
			auto torqueColor = GLColor<GLfloat, 4>(0.9f, 0.9f, 0.0f);	// yellow
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, torqueColor);
			glPushMatrix();
			{
				Vector rotAxis = Geometry::cross(Vrui::getUpDirection(), netTorque);
				Scalar rotAngle = Math::acos(netTorque/netTorqueMag * Vrui::getUpDirection());
				glMultMatrix(RotTransform(Rotation(rotAxis, rotAngle)));
				glDrawArrow(0.5f, 1.0f, 1.0f, Math::min(netTorqueMag, 25.0), 6);
			}
			glPopMatrix();
		}
		*/
	}
}

void VrProteinApp::frame() {
	/* Skip every other frame: */
	//frameSkip = !frameSkip;
	//if (frameSkip)
	//	return;

	/* Calculate the current time step: */
	double newFrameTime = Vrui::getApplicationTime();
	Scalar timeStep = Scalar(newFrameTime - lastFrameTime);
	lastFrameTime = newFrameTime;

	bool isStatsVisible = Vrui::getWidgetManager()->isVisible(statisticsDialog);

	if (isSimulating) {
		auto& ligand = drawMolecules.at(0);
		auto& protein = drawMolecules.at(1);
		// Calculate stuff
		latestSimResult = simulator.step(*ligand, *protein,	isCalculatingForces);

		// Update best solution for pocket
		int closestPocket = latestSimResult.closestPocket;
		if (closestPocket != -1) {
			if (simulator.compare(latestSimResult, bestSimResults.at(closestPocket))) {
				// Save result and move transparent marker
				bestSimResults.at(closestPocket) = latestSimResult;
				bestDrawMolecules.at(closestPocket)->SetState(ligand->GetState());
			}
			// Hide all solutions except the "active pocket" one
			for (auto& it : bestDrawMolecules)
				it.second->SetVisibility(false);
			bestDrawMolecules.at(closestPocket)->SetVisibility(true);
		}

		// Apply force to molecule
		if (isCalculatingForces) {
			if (latestSimResult.energy != 0)
				ligand->Step(latestSimResult.netForce * forceAttenuation,
						latestSimResult.netTorque * forceAttenuation, timeStep);
			else
				ligand->ResetForces();
		}

		// Draw statistics (update only if dialog is visible)
		if (isStatsVisible) {
			heuristicTextField->setValue(latestSimResult.energy);
			overlappingTextField->setValue(latestSimResult.overlappingAmount);
			closestPocketTextField->setValue(latestSimResult.closestPocket);
			meanDistanceTextField->setValue(latestSimResult.meanPocketDist);
			frameRateTextField->setValue(static_cast<int>(1/timeStep));
		}
		// widgets
		overlapWidget->setValue(latestSimResult.overlappingAmount);
		distanceWidget->setTitle(
				"Dist. " + protein->GetNameOfPocket(latestSimResult.closestPocket));
		distanceWidget->setValue(latestSimResult.meanPocketDist);
	}
	else {
		if (isStatsVisible) {
			heuristicTextField->setString("---");
			overlappingTextField->setString("---");
			closestPocketTextField->setString("---");
			meanDistanceTextField->setString("---");
			frameRateTextField->setValue(static_cast<int>(1/timeStep));
		}
	}
}

void VrProteinApp::debug() {
	/* Toggle DrawStyle:
	std::cout << "Debug: toggle DrawStyle" << std::endl;
	selectedMoleculeIdx = 1;
	if (drawMolecules.at(1)->GetDrawStyle() == DrawStyle::Surf)
		setDrawStyle(DrawStyle::Points);
	else
		setDrawStyle(DrawStyle::Surf);
	*/

	// Toggle transparency
	std::cout << "Debug: toggle Transparency" << std::endl;
	drawMolecules[0]->SetTransparency(true);
	//drawMolecules[1]->SetTransparency(true);
}

void VrProteinApp::setupExperiment(int experimentId) {
	std::cout << "Loading experiment " << experimentId << std::endl;
	const int maxPockets = 5;
	drawMolecules.clear();
	bestDrawMolecules.clear();
	switch(experimentId) {
	case 1:
		drawMolecules.push_back(CreateMolecule("1BU4/1BU4_2GP.pdb"));
		drawMolecules.push_back(CreateMolecule("1BU4/1BU4.pdb"));
		for(int i = 0; i < maxPockets; i++)
			bestDrawMolecules[i + 1] = (CreateMolecule("1BU4/1BU4_2GP.pdb"));
		break;
	case 2:
		drawMolecules.push_back(CreateMolecule("1STP/1STP_BTN.pdb"));
		drawMolecules.push_back(CreateMolecule("1STP/1STP.pdb"));
		for(int i = 0; i < maxPockets; i++)
			bestDrawMolecules[i + 1] = (CreateMolecule("1STP/1STP_BTN.pdb"));
		break;
	case 3:
		drawMolecules.push_back(CreateMolecule("3PTB/3PTB_BEN.pdb"));
		drawMolecules.push_back(CreateMolecule("3PTB/3PTB.pdb"));
		for(int i = 0; i < maxPockets; i++)
			bestDrawMolecules[i + 1] = (CreateMolecule("3PTB/3PTB_BEN.pdb"));
		break;
	case 4:
		drawMolecules.push_back(CreateMolecule("3VGC/3VGC_SRB.pdb"));
		drawMolecules.push_back(CreateMolecule("3VGC/3VGC.pdb"));
		for(int i = 0; i < maxPockets; i++)
			bestDrawMolecules[i + 1] = (CreateMolecule("3VGC/3VGC_SRB.pdb"));
		break;
	case 5:
		drawMolecules.push_back(CreateMolecule("1XIG/1XIG_XYL.pdb"));
		drawMolecules.push_back(CreateMolecule("1XIG/1XIG.pdb"));
		for(int i = 0; i < maxPockets; i++)
			bestDrawMolecules[i + 1] = (CreateMolecule("1XIG/1XIG_XYL.pdb"));
		break;
	default:
		throw std::runtime_error("Bad call to setupExperiment");
	}
	// Ligand
	drawMolecules.at(0)->SetColorStyle(ColorStyle::CPK);
	drawMolecules.at(0)->SetDrawStyle(DrawStyle::Surf);
	drawMolecules.at(0)->SetState(ONTransform::translateFromOriginTo(Point(20, 0, 0)));
	// Protein
	drawMolecules.at(1)->SetColorStyle(ColorStyle::Pockets);
	drawMolecules.at(1)->SetDrawStyle(DrawStyle::Surf);
	drawMolecules.at(1)->SetState(ONTransform::translateFromOriginTo(Point(-20, 0, 0)));
	drawMolecules.at(1)->Lock();
	// Transp
	for(auto& it : bestDrawMolecules) {
		it.second->SetColorStyle(ColorStyle::CPK);
		it.second->SetDrawStyle(DrawStyle::Surf);
		it.second->SetState(ONTransform::translateFromOriginTo(Point(20, 0, 0)));
		it.second->SetTransparency(true);
		it.second->SetVisibility(false);
		it.second->Lock();
	}

	//Reset SimResults
	auto defaultSimResult = Simulator::SimResult();
	defaultSimResult.energy = 999999;
	defaultSimResult.closestPocket = -1;
	defaultSimResult.meanPocketDist = 999999;
	bestSimResults.clear();
	for (int i = 0; i < maxPockets; i++)
		bestSimResults[i + 1] = defaultSimResult;

	centerDisplay();
}

void VrProteinApp::saveSolution() {
	if (Vrui::isMaster()) {
		/* Ensure file is open: */
		if (experimentFile == nullptr) {
			try {
				auto filename = TimeToString("Experiment %Y%m%d-%H%M.txt").c_str();
				experimentFile = unique_ptr<Misc::File>(new Misc::File(filename, "wt"));
				std::cout << "Created file " << filename << std::endl;
			}
			catch (Misc::File::OpenError&) {
				Vrui::showErrorMessage("Experiment", "Could not create experiment file.");
			}
		}
		/* Write to file if its opened: */
		if (experimentFile != nullptr) {
			auto message = TimeToString("** Solution saved at %H:%M:%S") + "\n";
			message += "Ligand: " + drawMolecules.at(0)->GetMolecule().source_filename + "\n"
					+ ONTransformToString(drawMolecules.at(0)->GetState());
			message += "Protein: " + drawMolecules.at(1)->GetMolecule().source_filename + "\n"
					+ ONTransformToString(drawMolecules.at(1)->GetState());
			if (isSimulating) {
				message += "Pocket:\n";
				message += "- Closest: " + std::to_string(latestSimResult.closestPocket) + "\n"
						+ "- Mean Pocket Dist: " + std::to_string(latestSimResult.meanPocketDist) + "\n"
						+ "- Energy: " + std::to_string(latestSimResult.energy) + "\n"
						+ "- Overlapping: "
						+ std::to_string(drawMolecules.at(0)->Intersects(*drawMolecules.at(1))) + "\n";
			}
			else
				std::cout << "WARNING: Simulation is OFF, pocket data not saved!" << std::endl;

			message += "\n";
			experimentFile->puts(message.c_str());
			fflush(experimentFile->getFilePtr());
			std::cout << message;
		}
	}
}

std::string VrProteinApp::ONTransformToString(const ONTransform& transform) const {
	const Scalar* p = transform.getTranslation().getComponents();
	const Scalar* q = transform.getRotation().getQuaternion();
	auto result = string_format("- Position: (%f, %f, %f)\n", p[0], p[1], p[2]);
	result += string_format("- Orientation: (%f, %f, %f, %f)\n", q[0], q[1], q[2], q[3]);
	return result;
}

/**************
 * UI methods:
 **************/

void VrProteinApp::createMainMenu(void) {
	mainMenuPopup = new PopupMenu("MainMenuPopup", Vrui::getWidgetManager());
	mainMenuPopup->setTitle("VR Protein App");

	auto mainMenu = new Menu("MainMenu", mainMenuPopup, false);

	auto centerDisplayButton = new Button("CenterDisplayButton", mainMenu, "Center Display");
	centerDisplayButton->getSelectCallbacks().add([](CallbackData* cbData, void* app) {
		static_cast<VrProteinApp*>(app)->centerDisplay();
	}, this);

	showSettingsDialogToggle = new ToggleButton("ShowSettingsDialogToggle", mainMenu,
			"Show Settings Dialog");
	showSettingsDialogToggle->getValueChangedCallbacks().add([](CallbackData* cbData, void* app) {
		auto _app = static_cast<VrProteinApp*>(app);
		/* Hide or show settings dialog based on toggle button state: */
		if (static_cast<ToggleButton::ValueChangedCallbackData*>(cbData)->set)
			Vrui::popupPrimaryWidget(_app->settingsDialog);
		else
			Vrui::popdownPrimaryWidget(_app->settingsDialog);
	}, this);

	showStatisticsDialogToggle = new ToggleButton("ShowStatisticsDialogToggle", mainMenu,
			"Show Statistics");
	showStatisticsDialogToggle->getValueChangedCallbacks().add([](CallbackData* cbData, void* app) {
		auto _app = static_cast<VrProteinApp*>(app);
		/* Hide or show statistics dialog based on toggle button state: */
		if (static_cast<ToggleButton::ValueChangedCallbackData*>(cbData)->set)
			Vrui::popupPrimaryWidget(_app->statisticsDialog);
		else
			Vrui::popdownPrimaryWidget(_app->statisticsDialog);
	}, this);

	showHudWidgetToggle = new ToggleButton("ShowHudWidgetToggle", mainMenu, "Show HUD");
	showHudWidgetToggle->getValueChangedCallbacks().add([](CallbackData* cbData, void* app) {
		auto _app = static_cast<VrProteinApp*>(app);
		/* Hide or show HUD dialog based on toggle button state: */
		if (static_cast<ToggleButton::ValueChangedCallbackData*>(cbData)->set) {
			Vrui::popupPrimaryWidget(_app->overlapWidget.get());
			Vrui::popupPrimaryWidget(_app->distanceWidget.get());
		}
		else {
			Vrui::popdownPrimaryWidget(_app->overlapWidget.get());
			Vrui::popdownPrimaryWidget(_app->distanceWidget.get());
		}
	}, this);

	mainMenu->manageChild();
}

void VrProteinApp::createSettingsDialog(void) {
	settingsDialog = new PopupWindow("SettingsDialog", Vrui::getWidgetManager(), "Settings Dialog");
	settingsDialog->setCloseButton(true);
	settingsDialog->getCloseCallbacks().add([](Misc::CallbackData* cbData, void* app) {
		/* Unset toggle button on main menu */
		static_cast<VrProteinApp*>(app)->showSettingsDialogToggle->setToggle(false);
	}, this);

	auto settings = new RowColumn("Settings", settingsDialog, false);

	// Molecule selector dropdown
	new Label("SelectedLabel", settings, "Selected molecule:");
	auto molItems = GetDropdownItemStrings();
	moleculeSelector = new DropdownBox("moleculeSelector", settings, molItems, false);
	moleculeSelector->getValueChangedCallbacks().add(this,
			&VrProteinApp::moleculeSelectorChangedCallback);
	moleculeSelector->manageChild();

	// Molecule Picker radio box
	new Label("LoadLabel", settings, "Load molecule:");
	std::vector<std::string> loadItems =
			{ "---", "alanin/alanin.pdb", "dna/dna.pdb", "1STP/1STP.pdb", "1STP/1STP_BTN.pdb",
					"1BU4/1BU4.pdb", "1BU4/1BU4_2GP.pdb", "3VGC/3VGC.pdb", "3VGC/3VGC_SRB.pdb",
					"1XIG/1XIG.pdb", "1XIG/1XIG_XYL.pdb", "test_1.pdb" };
	auto moleculeLoader = new DropdownBox("MoleculeLoader", settings, loadItems, false);
	moleculeLoader->getValueChangedCallbacks().add(this,
			&VrProteinApp::moleculeLoaderChangedCallback);
	moleculeLoader->manageChild();

	// Style picker radio box
	new Label("StyleLabel", settings, "Render style:");
	stylePicker = new RadioBox("StylePicker", settings, false);
	new ToggleButton("PointsBtn", stylePicker, "Points");
	new ToggleButton("SurfBtn", stylePicker, "Surf");
	stylePicker->getValueChangedCallbacks().add([](Misc::CallbackData* cbData, void* app) {
		auto _app = static_cast<VrProteinApp*>(app);
		auto _cbData = static_cast<RadioBox::ValueChangedCallbackData*>(cbData);
		auto idx = _app->stylePicker->getChildIndex(_cbData->newSelectedToggle);
		_app->setDrawStyle(static_cast<DrawStyle>(idx + 1), false);
	}, this);
	stylePicker->setSelectedToggle(static_cast<int>(selectedStyle) - 1); // default
	stylePicker->setSelectionMode(RadioBox::ALWAYS_ONE);
	stylePicker->manageChild();

	// Colors style picker
	new Label("ColorStyleLabel", settings, "Color style:");
	colorStylePicker = new RadioBox("ColorStylePicker", settings, false);
	new ToggleButton("NoneBtn", colorStylePicker, "None");
	new ToggleButton("AnaglyphBtn", colorStylePicker, "Anaglyph");
	new ToggleButton("CPKBtn", colorStylePicker, "CPK");
	new ToggleButton("PocketsBtn", colorStylePicker, "Pockets");
	colorStylePicker->getValueChangedCallbacks().add([](Misc::CallbackData* cbData, void* app) {
		auto _app = static_cast<VrProteinApp*>(app);
		auto _cbData = static_cast<RadioBox::ValueChangedCallbackData*>(cbData);
		auto idx = _app->colorStylePicker->getChildIndex(_cbData->newSelectedToggle);
		_app->setColorStyle(static_cast<ColorStyle>(idx), false);
	}, this);
	colorStylePicker->setSelectedToggle(static_cast<int>(selectedColorStyle)); // default
	colorStylePicker->setSelectionMode(RadioBox::ALWAYS_ONE);
	colorStylePicker->manageChild();

	settings->manageChild();
}

void VrProteinApp::createStatisticsDialog(void) {
	statisticsDialog = new PopupWindow("StatisticsDialog", Vrui::getWidgetManager(),
			"Simulation Statistics");
	statisticsDialog->setCloseButton(true);
	statisticsDialog->getCloseCallbacks().add([](Misc::CallbackData* cbData, void* app) {
		/* Unset toggle button on main menu */
		static_cast<VrProteinApp*>(app)->showStatisticsDialogToggle->setToggle(false);
	}, this);

	auto statistics = new RowColumn("Statistics", statisticsDialog, false);
	statistics->setNumMinorWidgets(3);

	// Heuristic value
	new Label("HeuristicLabel", statistics, "L-J potential:");
	heuristicTextField = new TextField("HeuristicTextField", statistics, 12, true);
	new Label("HeuristicUnitsLabel1", statistics, "(J)");

	// Is Overlapping
	new Label("OverlappingLabel", statistics, "Max overlap:");
	overlappingTextField = new TextField("overlappingTextField", statistics, 12, true);
	new Label("HeuristicUnitsLabel2", statistics, "(A)");

	// Closest pocket
	new Label("ClosestPocketLabel", statistics, "Closest pocket:");
	closestPocketTextField = new TextField("ClosestPocketTextField", statistics, 12, true);
	new Label("EmptyLabel1", statistics, "");

	// Mean distance
	new Label("MeanDistanceLabel", statistics, "Mean distance:");
	meanDistanceTextField = new TextField("MeanDistanceTextField", statistics, 12, true);
	new Label("HeuristicUnitsLabel3", statistics, "(A)");

	// Mean distance
	new Label("FrameRateLabel", statistics, "Frame Rate:");
	frameRateTextField = new TextField("FrameRateTextField", statistics, 12, true);
	new Label("EmptyLabel2", statistics, "");

	// Do realtime statistics
	simulateBtn = new ToggleButton("SimulateBtn", statistics, "Simulate");
	simulateBtn->setToggle(isSimulating);
	simulateBtn->getValueChangedCallbacks().add([](CallbackData* cbData, void* app) {
		auto _app = static_cast<VrProteinApp*>(app);
		auto _cbData = static_cast<ToggleButton::ValueChangedCallbackData*>(cbData);
		/* Set simulation status based on toggle button state: */
		_app->toggleSimulation(_cbData->set, false);
	}, this);

	// Calculate forces
	calculateForcesBtn = new ToggleButton("CalculateForcesBtn", statistics, "Calc. forces");
	calculateForcesBtn->setToggle(isCalculatingForces);
	calculateForcesBtn->getValueChangedCallbacks().add([](CallbackData* cbData, void* app) {
		auto _app = static_cast<VrProteinApp*>(app);
		auto _cbData = static_cast<ToggleButton::ValueChangedCallbackData*>(cbData);
		/* Set forces based on toggle button state: */
		_app->forceAttenuation = 1;
		_app->toggleForces(_cbData->set, false);
	}, this);

	statistics->manageChild();
}

/* Returns a list of currently loaded molecule names and indexes.
 * Format is: [0: alanin.pdb, 1: BrH.pdb, ...] */
std::vector<std::string> VrProteinApp::GetDropdownItemStrings() const {
	auto items = std::vector<std::string>();
	if (drawMolecules.size() == 0) {
		std::cout << "Warning: No molecules loaded" << std::endl;
		items.push_back("");
	}

	for (unsigned int i = 0; i < drawMolecules.size(); i++) {
		items.push_back(std::to_string(i) + ": " + drawMolecules.at(i)->GetName());
	}
	return items;
}

/* Center display on origin */
void VrProteinApp::centerDisplay() {
	std::cout << "Centering display." << std::endl;
	Vrui::setNavigationTransformation(Point::origin, Scalar(40));
}

/* Toggle simulation of molecules */
void VrProteinApp::toggleSimulation(bool simulate, bool refreshUI /* = true */) {
	std::cout << "Toggling simulation to " << simulate << std::endl;
	isSimulating = simulate;
	if (refreshUI) {
		simulateBtn->setToggle(simulate);
	}
}

/* Toggle calculation of forces and torques */
void VrProteinApp::toggleForces(bool calculateForces, bool refreshUI /* = true */) {
	std::cout << "Toggling force calc. to " << calculateForces << std::endl;
	isCalculatingForces = calculateForces;
	// Turn on simulation if it was disabled
	if (calculateForces && !isSimulating)
		toggleSimulation(true);
	if (!calculateForces)
		drawMolecules.at(0)->ResetForces();
	if (refreshUI) {
		calculateForcesBtn->setToggle(calculateForces);
	}
}

void VrProteinApp::setForceAttenuation(Scalar factor) {
	forceAttenuation = factor;
	if (factor < 0.05 && isCalculatingForces) {
		toggleForces(false);
	}
	else if (factor > 0.05 && !isCalculatingForces) {
		toggleForces(true);
	}
}

void VrProteinApp::setColorStyle(ColorStyle newStyle, bool refreshUI /* = true */) {
	std::cout << "Changing color style to " << static_cast<int>(newStyle) << std::endl;
	selectedColorStyle = newStyle;
	drawMolecules.at(selectedMoleculeIdx)->SetColorStyle(selectedColorStyle);
	if (refreshUI) {
		colorStylePicker->setSelectedToggle(static_cast<int>(newStyle));
	}
}

void VrProteinApp::setDrawStyle(DrawStyle newStyle, bool refreshUI /* = true */) {
	std::cout << "Changing draw style to " << static_cast<int>(newStyle) << std::endl;
	selectedStyle = newStyle;
	drawMolecules.at(selectedMoleculeIdx)->SetDrawStyle(selectedStyle);
	if (refreshUI) {
		stylePicker->setSelectedToggle(static_cast<int>(newStyle));
	}
}

void VrProteinApp::refreshSettingsDialog(bool rebuildMoleculeSelector /* = true */) {
	// update selector item labels
	if (rebuildMoleculeSelector) {
		moleculeSelector->clearItems();
		for (auto& str : GetDropdownItemStrings()) {
			moleculeSelector->addItem(str.c_str());
		}
	}
	moleculeSelector->setSelectedItem(selectedMoleculeIdx);
	// moleculeLoader
	auto ml = dynamic_cast<DropdownBox*>(settingsDialog->findDescendant("Settings/MoleculeLoader"));
	ml->setSelectedItem(0);
	// stylePicker
	selectedStyle = drawMolecules.at(selectedMoleculeIdx)->GetDrawStyle();
	auto sp = dynamic_cast<RadioBox*>(settingsDialog->findDescendant("Settings/StylePicker"));
	sp->setSelectedToggle(static_cast<int>(selectedStyle) - 1);
	// colorStylePicker
	selectedColorStyle = drawMolecules.at(selectedMoleculeIdx)->GetColorStyle();
	colorStylePicker->setSelectedToggle(static_cast<int>(selectedColorStyle));
}


/* Selected a molecule for editing in settings dialog */
void VrProteinApp::moleculeSelectorChangedCallback(DropdownBox::ValueChangedCallbackData* cbData) {
	selectedMoleculeIdx = cbData->newSelectedItem;
	refreshSettingsDialog(false);
}

/* Load a new molecule */
void VrProteinApp::moleculeLoaderChangedCallback(DropdownBox::ValueChangedCallbackData* cbData) {
	std::string name = cbData->getItem();
	if (name == "---")
		return;
	std::cout << "Selected " << name << " from picker." << std::endl;

	drawMolecules.at(selectedMoleculeIdx) = CreateMolecule(name);
	// reset all draggers, just in case
	for (auto& d : moleculeDraggers)
		d->Reset();
	refreshSettingsDialog();
}

void VrProteinApp::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData) {
	Application::toolCreationCallback(cbData);
	/* Check if the new tool is a dragging tool: */
	Vrui::DraggingTool* tool = dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if (tool != nullptr) {
		/* Create an atom dragger object and associate it with the new tool: */
		moleculeDraggers.push_back(unique_ptr<MoleculeDragger>(new MoleculeDragger(tool, this)));
	}
}

void VrProteinApp::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData) {
	/* Check if the to-be-destroyed tool is a dragging tool: */
	Vrui::DraggingTool* tool = dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if (tool != nullptr) {
		/* Find the molecule dragger associated with the tool in the list: */
		std::vector<unique_ptr<MoleculeDragger>>::iterator mdIt;
		for (mdIt = moleculeDraggers.begin();
				mdIt != moleculeDraggers.end() && (*mdIt)->getTool() != tool; ++mdIt)
			;
		if (mdIt != moleculeDraggers.end()) {
			/* Remove the molecule dragger: */
			moleculeDraggers.erase(mdIt);
		}
	}
}

/* Load a molecule from file */
unique_ptr<DrawMolecule> VrProteinApp::CreateMolecule(const std::string& fileName) const {
	unique_ptr<Molecule> m = PDBImporter::ParsePDB("./datasets/" + fileName);
	auto drawMolecule = unique_ptr<DrawMolecule>(new DrawMolecule(move(m)));

	drawMolecule->SetDrawStyle(selectedStyle);
	drawMolecule->SetColorStyle(selectedColorStyle);

	return drawMolecule;
}

/* Find index of molecule by its name. Throws on failure. */
int VrProteinApp::IndexOfMolecule(const std::string& moleculeName) const {
	for (unsigned int i = 0; i < drawMolecules.size(); i++) {
		if (drawMolecules.at(i)->GetName() == moleculeName) {
			return i;
		}
	}
	throw std::runtime_error("Molecule not found: " + moleculeName);
}

}

/* Create and execute an application object: */
int main(int argc, char* argv[]) {
	// Generate debug info on crash.
	struct sigaction sigact;
	sigact.sa_sigaction = crit_err_hdlr;
	sigact.sa_flags = SA_RESTART | SA_SIGINFO;
	if (sigaction(SIGSEGV, &sigact, (struct sigaction *) NULL) != 0) {
		fprintf(stderr, "error setting signal handler for %d (%s)\n",
		SIGSEGV, strsignal(SIGSEGV));
		exit(EXIT_FAILURE);
	}

	try {
		VrProtein::VrProteinApp app(argc, argv);
		app.run();
	}
	catch (std::runtime_error &err) {
		std::cerr << "Terminated program due to exception: " << err.what() << std::endl;
		return 1;
	}

	return 0;
}
