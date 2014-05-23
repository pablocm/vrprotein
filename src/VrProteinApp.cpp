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

#include <iostream>
//#include <GL/GLModels.h>
//#include <GL/GLMaterialTemplates.h>
//#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Button.h>
#include <GLMotif/Label.h>
#include <GLMotif/Menu.h>
#include <GLMotif/RowColumn.h>
#include <Vrui/CoordinateManager.h>
#include "utils/backtrace.h"
#include "VrProteinApp.h"
#include "DrawMolecule.h"
#include "HudWidget.h"
#include "Molecule.h"
#include "MoleculeDragger.h"
#include "PDBImporter.h"

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
			selectedUseColor(true),
			selectedMoleculeIdx(0),
			isSimulating(false),
			isCalculatingForces(true) {
	/* load molecule data */
	drawMolecules.push_back(LoadMolecule("alanin.pdb"));
	drawMolecules.push_back(LoadMolecule("alanin.pdb"));

	/* Move them away */
	drawMolecules[0]->SetState(ONTransform::translateFromOriginTo(Point(-10, 0, 0)));
	drawMolecules[1]->SetState(ONTransform::translateFromOriginTo(Point(10, 0, 0)));

	/* Set the navigation transformation to show the entire scene: */
	centerDisplay();
	/* Set the navigational coordinate system unit: */
	Vrui::getCoordinateManager()->setUnit(Geometry::LinearUnit(Geometry::LinearUnit::ANGSTROM, 1));

	/* Create the program's user interface: */
	mainMenu = createMainMenu();
	Vrui::setMainMenu(mainMenu);
	settingsDialog = createSettingsDialog();
	statisticsDialog = createStatisticsDialog();
	hudWidget = new HudWidget("HudWidget", Vrui::getWidgetManager(), "L-J Potential");

	/* Tell Vrui to run in a continuous frame sequence: */
	Vrui::updateContinuously();
}

void VrProteinApp::display(GLContextData& contextData) const {
	// Draw domain box
	domainBox.glRenderAction(contextData);
	// Draw molecules
	for (auto& m : drawMolecules) {
		m->glRenderAction(contextData);
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
	if (isSimulating) {
		// Calculate stuff
		auto overlappingAmount = drawMolecules[0]->Intersects(*drawMolecules[1]);
		simResult = simulator.step(*drawMolecules[0], *drawMolecules[1], isCalculatingForces);

		// Apply force to molecule
		if (isCalculatingForces && simResult.energy != 0) {
			auto t = drawMolecules[0]->GetState();
			auto t2 = ONTransform(simResult.netForce.normalize() * 0.04,
					Rotation(simResult.netTorque, 0.008));
			drawMolecules[0]->SetState(t * t2);
		}

		// Draw statistics
		heuristicTextField->setValue(simResult.energy); //simResult.netForce.mag()); //
		overlappingTextField->setValue(overlappingAmount);
		hudWidget->setValue(simResult.energy); //simResult.netForce.mag()); //

	}
	else {
		heuristicTextField->setString("---");
		overlappingTextField->setString("---");
	}
}

/**************
 * UI methods:
 **************/

PopupMenu* VrProteinApp::createMainMenu(void) {
	auto mainMenuPopup = new PopupMenu("MainMenuPopup", Vrui::getWidgetManager());
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
		if (static_cast<ToggleButton::ValueChangedCallbackData*>(cbData)->set)
			Vrui::popupPrimaryWidget(_app->statisticsDialog);
		else
			Vrui::popdownPrimaryWidget(_app->statisticsDialog);
	}, this);

	mainMenu->manageChild();

	return mainMenuPopup;
}

PopupWindow* VrProteinApp::createSettingsDialog(void) {
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
	auto moleculeLoader = new RadioBox("MoleculeLoader", settings, false);
	new ToggleButton("AlaninBtn", moleculeLoader, "alanin.pdb");
	new ToggleButton("DNABtn", moleculeLoader, "dna.pdb");
	new ToggleButton("BrHBtn", moleculeLoader, "brH.pdb");
	new ToggleButton("1STPBtn", moleculeLoader, "1STP.pdb");
	new ToggleButton("1STP_BTNBtn", moleculeLoader, "1STP_BTN.pdb");
	moleculeLoader->getValueChangedCallbacks().add(this,
			&VrProteinApp::moleculeLoaderChangedCallback);
	moleculeLoader->setSelectionMode(RadioBox::ALWAYS_ONE);
	moleculeLoader->setSelectedToggle(0); // Alanin default
	moleculeLoader->manageChild();

	// Style picker radio box
	new Label("StyleLabel", settings, "Render style:");
	auto stylePicker = new RadioBox("StylePicker", settings, false);
	new ToggleButton("PointsBtn", stylePicker, "Points");
	new ToggleButton("SurfBtn", stylePicker, "Surf");
	stylePicker->getValueChangedCallbacks().add(this, &VrProteinApp::stylePickerChangedCallback);
	stylePicker->setSelectedToggle(1); // Surf default
	stylePicker->setSelectionMode(RadioBox::ALWAYS_ONE);
	stylePicker->manageChild();

	// Use colors toggle
	auto colorBtn = new ToggleButton("ColorBtn", settings, "Use colors");
	colorBtn->setToggle(true); // UseColor default
	colorBtn->getValueChangedCallbacks().add(this, &VrProteinApp::colorToggleChangedCallback);

	settings->manageChild();

	return settingsDialog;
}

PopupWindow* VrProteinApp::createStatisticsDialog(void) {
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
	new Label("HeuristicUnitsLabel", statistics, "(J)");

	// Is Overlapping
	new Label("overlappingLabel", statistics, "Max overlap:");
	overlappingTextField = new TextField("overlappingTextField", statistics, 12, true);
	new Label("HeuristicUnitsLabel", statistics, "(A)");

	// Do realtime statistics
	auto simulateBtn = new ToggleButton("SimulateBtn", statistics, "Simulate");
	simulateBtn->setToggle(isSimulating);
	simulateBtn->getValueChangedCallbacks().add(this, &VrProteinApp::simulateToggleChangedCallback);

	// Calculate forces
	auto calculateForcesBtn = new ToggleButton("CalculateForcesBtn", statistics, "Calc. forces");
	calculateForcesBtn->setToggle(isCalculatingForces);
	calculateForcesBtn->getValueChangedCallbacks().add(this,
			&VrProteinApp::calculateForcesToggleChangedCallback);

	statistics->manageChild();

	return statisticsDialog;
}

/* Returns a list of currently loaded molecule names and indexes.
 * Format is: [0: alanin.pdb, 1: BrH.pdb, ...] */
std::vector<std::string> VrProteinApp::GetDropdownItemStrings() const {
	auto items = std::vector<std::string>();
	for (unsigned int i = 0; i < drawMolecules.size(); i++) {
		items.push_back(std::to_string(i) + ": " + drawMolecules[i]->GetName());
	}
	return items;
}

/* Center display on origin */
void VrProteinApp::centerDisplay() {
	std::cout << "Centering display." << std::endl;
	Vrui::setNavigationTransformation(Point::origin, Scalar(40));
}

/* Selected a molecule for editing in settings dialog */
void VrProteinApp::moleculeSelectorChangedCallback(DropdownBox::ValueChangedCallbackData* cbData) {
	selectedMoleculeIdx = cbData->newSelectedItem;
}

/* Load a new molecule */
void VrProteinApp::moleculeLoaderChangedCallback(RadioBox::ValueChangedCallbackData* cbData) {
	std::string name = cbData->newSelectedToggle->getString();
	std::cout << "Selected " << name << " from picker." << std::endl;

	drawMolecules[selectedMoleculeIdx] = LoadMolecule(name);
	// update dropdown label
	moleculeSelector->clearItems();
	for (auto& str : GetDropdownItemStrings()) {
		moleculeSelector->addItem(str.c_str());
	}
	moleculeSelector->setSelectedItem(selectedMoleculeIdx);

	// reset all draggers, just in case
	for (auto& d : moleculeDraggers)
		d->Reset();
}

/* Toggle draw style */
void VrProteinApp::stylePickerChangedCallback(RadioBox::ValueChangedCallbackData* cbData) {
	std::string style = cbData->newSelectedToggle->getString();
	std::cout << "Selected style " << style << " from picker." << std::endl;
	if (style == "Points") {
		selectedStyle = DrawStyle::Points;

		drawMolecules[selectedMoleculeIdx]->SetDrawStyle(DrawStyle::Points);
	}
	else if (style == "Surf") {
		selectedStyle = DrawStyle::Surf;
		drawMolecules[selectedMoleculeIdx]->SetDrawStyle(DrawStyle::Surf);
	}
	else
		throw "Unknown style " + style;
}

/* Toggle use of color in molecules */
void VrProteinApp::colorToggleChangedCallback(ToggleButton::ValueChangedCallbackData* cbData) {
	selectedUseColor = cbData->set;
	drawMolecules[selectedMoleculeIdx]->SetColorStyle(cbData->set);
}

/* Toggle simulation of molecules */
void VrProteinApp::simulateToggleChangedCallback(ToggleButton::ValueChangedCallbackData* cbData) {
	isSimulating = cbData->set;
}

/* Toggle calculation of forces and torques */
void VrProteinApp::calculateForcesToggleChangedCallback(
		ToggleButton::ValueChangedCallbackData* cbData) {
	isCalculatingForces = cbData->set;
}

void VrProteinApp::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData) {
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
unique_ptr<DrawMolecule> VrProteinApp::LoadMolecule(const std::string& fileName) {
	unique_ptr<Molecule> m = PDBImporter::ParsePDB("./datasets/" + fileName);
	auto drawMolecule = unique_ptr<DrawMolecule>(new DrawMolecule(move(m)));

	drawMolecule->SetDrawStyle(selectedStyle);
	drawMolecule->SetColorStyle(selectedUseColor);

	Point center;
	if (drawMolecule) {
		center = drawMolecule->GetCenter();
	}
	drawMolecule->SetState(ONTransform::translateToOriginFrom(center));
	return drawMolecule;
}

/* Find index of molecule by its name. Throws on failure. */
int VrProteinApp::IndexOfMolecule(const std::string& moleculeName) const {
	for (unsigned int i = 0; i < drawMolecules.size(); i++) {
		if (drawMolecules[i]->GetName() == moleculeName) {
			return i;
		}
	}
	throw new std::runtime_error("Molecule not found: " + moleculeName);
}

}

/* Create and execute an application object: */
int main(int argc, char* argv[]) {
	signal(SIGSEGV, handler);	// Generate debug info on crash.
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
