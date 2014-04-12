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

#include <memory>
#include <iostream>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/DropdownBox.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DraggingToolAdapter.h>
#include "utils/backtrace.h"
#include "AffineSpace.h"
#include "PDBImporter.h"
#include "Molecule.h"
#include "DrawMolecule.h"
#include "DomainBox.h"

using namespace VrProtein;
using std::unique_ptr;

class VrProteinApp: public Vrui::Application {
public:
	VrProteinApp(int& argc, char**& argv);

	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void frame();

private:
	/* Embedded classes: */
	class MoleculeDragger: public Vrui::DraggingToolAdapter { // Class to drag molecules
	private:
		VrProteinApp* application;
		bool dragging;
		int moleculeIdx;
		ONTransform dragTransform;

	public:
		MoleculeDragger(Vrui::DraggingTool* sTool, VrProteinApp* sApplication);
		virtual void dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData);
		virtual void dragCallback(Vrui::DraggingTool::DragCallbackData* cbData);
		virtual void dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData);
		void Reset();	// halt dragging right away
	};

	friend class MoleculeDragger;

	// Private fields
	DomainBox domainBox;
	std::vector<unique_ptr<DrawMolecule>> drawMolecules;
	DrawStyle selectedStyle;
	bool selectedUseColor;
	int selectedMoleculeIdx;
	// Private methods
	unique_ptr<DrawMolecule> LoadMolecule(const std::string& fileName);
	void SetDrawStyle(DrawStyle style);
	int IndexOfMolecule(const std::string& moleculeName) const;
	std::vector<std::string> GetDropdownItemStrings() const;
	// Tool items
	std::vector<unique_ptr<MoleculeDragger>> moleculeDraggers;
	// UI Items
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	GLMotif::ToggleButton* showSettingsDialogToggle;
	GLMotif::PopupWindow* settingsDialog; // The settings dialog
	GLMotif::DropdownBox* moleculeSelector;	// dropdown for molecule selector
	// UI Constructors
	GLMotif::PopupMenu* createMainMenu(void);
	GLMotif::PopupWindow* createSettingsDialog(void);
	// UI Callbacks
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void showSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void settingsDialogCloseCallback(Misc::CallbackData* cbData);
	void moleculeSelectorChangedCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void moleculeLoaderChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void stylePickerChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void colorToggleChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	// Tool Callbacks
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
};

/******************************
 Methods of class MoleculeDragger:
 ******************************/

VrProteinApp::MoleculeDragger::MoleculeDragger(Vrui::DraggingTool* sTool,
		VrProteinApp* sApplication) :
			Vrui::DraggingToolAdapter(sTool),
			application(sApplication),
			dragging(false),
			moleculeIdx(-1) {
}

void VrProteinApp::MoleculeDragger::dragStartCallback(
		Vrui::DraggingTool::DragStartCallbackData* cbData) {
	/* Find the picked atom: */
	moleculeIdx = -1;
	if (cbData->rayBased) {
		std::cout << "Checking ray intersect" << std::endl;
		for (unsigned int i = 0; i < application->drawMolecules.size(); i++) {
			if (application->drawMolecules[i]->Intersects(cbData->ray)) {
				moleculeIdx = i;
				break;
			}
		}
	}
	else {
		Point point = cbData->startTransformation.getOrigin();
		std::cout << "Checking intersect at ";
		std::cout << point[0] << ", " << point[1] << ", " << point[2] << std::endl;
		for (unsigned int i = 0; i < application->drawMolecules.size(); i++) {
			if (application->drawMolecules[i]->Intersects(point)) {
				moleculeIdx = i;
				break;
			}
		}
	}

	/* Try locking the atom: */
	if (moleculeIdx >= 0) {
		if (application->drawMolecules[moleculeIdx]->Lock()) {
			std::cout << "Grabbed molecule." << std::endl;
			dragging = true;

			/* Calculate the initial transformation from the dragger to the dragged atom: */
			dragTransform = ONTransform(cbData->startTransformation.getTranslation(),
					cbData->startTransformation.getRotation());
			dragTransform.doInvert();
			dragTransform *= application->drawMolecules[moleculeIdx]->GetState();
		}
		else
			std::cout << "Molecule is locked by another dragger." << std::endl;
	}
	else
		std::cout << "Nothing to grab at this location." << std::endl;
}

void VrProteinApp::MoleculeDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData) {
	if (dragging) {
		/* Apply the dragging transformation to the dragged atom: */
		ONTransform transform = ONTransform(cbData->currentTransformation.getTranslation(),
				cbData->currentTransformation.getRotation());
		transform *= dragTransform;
		application->drawMolecules[moleculeIdx]->SetState(transform);
	}
}

void VrProteinApp::MoleculeDragger::dragEndCallback(
		Vrui::DraggingTool::DragEndCallbackData* cbData) {
	if (dragging) {
		std::cout << "Released molecule." << std::endl;
		/* Release the previously dragged atom: */
		application->drawMolecules[moleculeIdx]->Unlock();
		moleculeIdx = -1;
		dragging = false;
	}
}

void VrProteinApp::MoleculeDragger::Reset() {
	if (dragging) {
		std::cout << "Dragger reset." << std::endl;
		application->drawMolecules[moleculeIdx]->Unlock();
		moleculeIdx = -1;
		dragging = false;
	}
}

/******************************
 Methods of class VrProteinApp:
 ******************************/

VrProteinApp::VrProteinApp(int& argc, char**& argv) :
			Vrui::Application(argc, argv),
			selectedStyle(DrawStyle::Surf),
			selectedUseColor(true),
			selectedMoleculeIdx(0) {
	/* load molecule data */
	drawMolecules.push_back(LoadMolecule("alanin.pdb"));
	drawMolecules.push_back(LoadMolecule("alanin.pdb"));

	/* Set the navigation transformation to show the entire scene: */
	centerDisplayCallback(nullptr);

	/* Create the program's user interface: */
	mainMenu = createMainMenu();
	Vrui::setMainMenu(mainMenu);
	settingsDialog = createSettingsDialog();
}

void VrProteinApp::display(GLContextData& contextData) const {
	// Draw domain box
	domainBox.glRenderAction(contextData);
	// Draw molecules
	for (auto& m : drawMolecules) {
		m->glRenderAction(contextData);
	}
}

void VrProteinApp::frame() {
}

/**************
 * UI methods:
 **************/

GLMotif::PopupMenu* VrProteinApp::createMainMenu(void) {
	GLMotif::PopupMenu* mainMenuPopup = new GLMotif::PopupMenu("MainMenuPopup",
			Vrui::getWidgetManager());
	mainMenuPopup->setTitle("VR Protein App");

	GLMotif::Menu* mainMenu = new GLMotif::Menu("MainMenu", mainMenuPopup, false);

	GLMotif::Button* centerDisplayButton = new GLMotif::Button("CenterDisplayButton", mainMenu,
			"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this, &VrProteinApp::centerDisplayCallback);

	showSettingsDialogToggle = new GLMotif::ToggleButton("ShowSettingsDialogToggle", mainMenu,
			"Show Settings Dialog");
	showSettingsDialogToggle->getValueChangedCallbacks().add(this,
			&VrProteinApp::showSettingsDialogCallback);

	mainMenu->manageChild();

	return mainMenuPopup;
}

GLMotif::PopupWindow* VrProteinApp::createSettingsDialog(void) {
	settingsDialog = new GLMotif::PopupWindow("SettingsDialog", Vrui::getWidgetManager(),
			"Settings Dialog");
	settingsDialog->setCloseButton(true);
	settingsDialog->getCloseCallbacks().add(this, &VrProteinApp::settingsDialogCloseCallback);

	GLMotif::RowColumn* settings = new GLMotif::RowColumn("Settings", settingsDialog, false);

	// Molecule selector dropdown
	new GLMotif::Label("SelectedLabel", settings, "Selected molecule:");
	auto molItems = GetDropdownItemStrings();
	moleculeSelector = new GLMotif::DropdownBox("moleculeSelector", settings, molItems, false);
	moleculeSelector->getValueChangedCallbacks().add(this,
			&VrProteinApp::moleculeSelectorChangedCallback);
	moleculeSelector->manageChild();

	// Molecule Picker radio box
	new GLMotif::Label("LoadLabel", settings, "Load molecule:");
	auto moleculeLoader = new GLMotif::RadioBox("MoleculeLoader", settings, false);
	new GLMotif::ToggleButton("AlaninBtn", moleculeLoader, "alanin.pdb");
	new GLMotif::ToggleButton("DNABtn", moleculeLoader, "dna.pdb");
	new GLMotif::ToggleButton("BrHBtn", moleculeLoader, "brH.pdb");
	moleculeLoader->getValueChangedCallbacks().add(this,
			&VrProteinApp::moleculeLoaderChangedCallback);
	moleculeLoader->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	moleculeLoader->setSelectedToggle(0); // Alanin default
	moleculeLoader->manageChild();

	// Style picker radio box
	new GLMotif::Label("StyleLabel", settings, "Render style:");
	auto stylePicker = new GLMotif::RadioBox("StylePicker", settings, false);
	new GLMotif::ToggleButton("PointsBtn", stylePicker, "Points");
	new GLMotif::ToggleButton("SurfBtn", stylePicker, "Surf");
	stylePicker->getValueChangedCallbacks().add(this, &VrProteinApp::stylePickerChangedCallback);
	stylePicker->setSelectedToggle(1); // Surf default
	stylePicker->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	stylePicker->manageChild();

	// Use colors toggle
	auto colorBtn = new GLMotif::ToggleButton("ColorBtn", settings, "Use colors");
	colorBtn->setToggle(true); // UseColor default
	colorBtn->getValueChangedCallbacks().add(this, &VrProteinApp::colorToggleChangedCallback);

	settings->manageChild();

	return settingsDialog;
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

/* Center display on currently loaded molecule */
void VrProteinApp::centerDisplayCallback(Misc::CallbackData* cbData) {
	std::cout << "Centering display." << std::endl;
	Vrui::setNavigationTransformation(Point::origin, Scalar(40));
}

void VrProteinApp::showSettingsDialogCallback(
		GLMotif::ToggleButton::ValueChangedCallbackData* cbData) {
	/* Hide or show settings dialog based on toggle button state: */
	if (cbData->set) {
		/* Pop up the settings dialog: */
		Vrui::popupPrimaryWidget(settingsDialog);
	}
	else
		Vrui::popdownPrimaryWidget(settingsDialog);
}

void VrProteinApp::settingsDialogCloseCallback(Misc::CallbackData* cbData) {
	showSettingsDialogToggle->setToggle(false);
}

/* Selected a new molecule */
void VrProteinApp::moleculeSelectorChangedCallback(
		GLMotif::DropdownBox::ValueChangedCallbackData* cbData) {
	selectedMoleculeIdx = cbData->newSelectedItem;
}

/* Load a new molecule */
void VrProteinApp::moleculeLoaderChangedCallback(
		GLMotif::RadioBox::ValueChangedCallbackData* cbData) {
	std::string name = cbData->newSelectedToggle->getString();
	std::cout << "Selected " << name << " from picker." << std::endl;

	drawMolecules[selectedMoleculeIdx] = LoadMolecule(name);
	// update dropdown label
	// TODO: ARREGLAR ESTO
	//auto dropdownItem = const_cast<GLMotif::Button*>(static_cast<const GLMotif::Button*>
	//						(moleculeSelector->getItemWidget(selectedMoleculeIdx)));
	//dropdownItem->setString(name.c_str());
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
void VrProteinApp::stylePickerChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData) {
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
void VrProteinApp::colorToggleChangedCallback(
		GLMotif::ToggleButton::ValueChangedCallbackData* cbData) {
	selectedUseColor = cbData->set;
	drawMolecules[selectedMoleculeIdx]->SetColorStyle(cbData->set);
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
	unique_ptr<Molecule> m = PDBImporter::ParsePDB(fileName);
	auto drawMolecule = unique_ptr<DrawMolecule>(new DrawMolecule(move(m)));

	drawMolecule->SetDrawStyle(selectedStyle);
	drawMolecule->SetColorStyle(selectedUseColor);

	float x = 0, y = 0, z = 0;
	if (drawMolecule) {
		drawMolecule->GetCenter(x, y, z);
	}
	drawMolecule->SetState(ONTransform::translateToOriginFrom(Point(x, y, z)));
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

/* Create and execute an application object: */
int main(int argc, char* argv[]) {
	signal(SIGSEGV, handler);	// Generate debug info on crash.
	try {
		VrProteinApp app(argc, argv);
		app.run();
	}
	catch (std::runtime_error &err) {
		std::cerr << "Terminated program due to exception: " << err.what() << std::endl;
		return 1;
	}

	return 0;
}
