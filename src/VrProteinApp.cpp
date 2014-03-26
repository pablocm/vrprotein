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
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
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
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include "utils/backtrace.h"
#include "PDBImporter.h"
#include "Molecule.h"
#include "DrawMolecule.h"

using std::unique_ptr;

class VrProteinApp: public Vrui::Application {
public:
	VrProteinApp(int& argc, char**& argv);

	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void frame();

private:
	// Private fields
	unique_ptr<DrawMolecule> drawMolecule;
	DrawStyle selectedStyle;
	bool selectedUseColor;

	// Private methods
	void LoadMolecule(const std::string& fileName);
	void SetDrawStyle(DrawStyle style);
	// UI Items
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	GLMotif::ToggleButton* showSettingsDialogToggle;
	GLMotif::PopupWindow* settingsDialog; // The settings dialog
	// UI Constructors
	GLMotif::PopupMenu* createMainMenu(void);
	GLMotif::PopupWindow* createSettingsDialog(void);
	// UI Callbacks
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void showSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void settingsDialogCloseCallback(Misc::CallbackData* cbData);
	void moleculePickerChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void stylePickerChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void colorToggleChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
};


/******************************
 Methods of class VrProteinApp:
 ******************************/

VrProteinApp::VrProteinApp(int& argc, char**& argv) :
		Vrui::Application(argc, argv) {
	/* Set defaults */
	selectedStyle = DrawStyle::Surf;
	selectedUseColor = true;

	/* load molecule data */
	LoadMolecule("alanin.pdb");

	/* Set the navigation transformation to show the entire scene: */
	centerDisplayCallback(nullptr);

	/* Create the program's user interface: */
	mainMenu = createMainMenu();
	Vrui::setMainMenu(mainMenu);
	settingsDialog = createSettingsDialog();
}

void VrProteinApp::display(GLContextData& contextData) const {
	// Draw the molecule
	drawMolecule->Draw(contextData);
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

GLMotif::PopupWindow* VrProteinApp::createSettingsDialog(void)
	{
	//const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();

	settingsDialog=new GLMotif::PopupWindow("SettingsDialog",Vrui::getWidgetManager(),"Settings Dialog");
	settingsDialog->setCloseButton(true);
	//settingsDialog->setResizableFlags(false,false);
	settingsDialog->getCloseCallbacks().add(this,&VrProteinApp::settingsDialogCloseCallback);

	GLMotif::RowColumn* settings=new GLMotif::RowColumn("Settings",settingsDialog,false);
	//settings->setNumMinorWidgets(2);

	new GLMotif::Label("MoleculeLabel",settings,"Load molecule:");

	// Molecule Picker radio box
	auto moleculePicker = new GLMotif::RadioBox("MoleculePicker", settings, false);
	new GLMotif::ToggleButton("AlaninBtn", moleculePicker, "alanin.pdb");
	new GLMotif::ToggleButton("DNABtn", moleculePicker, "dna.pdb");
	new GLMotif::ToggleButton("BrHBtn", moleculePicker, "brH.pdb");
	moleculePicker->getValueChangedCallbacks().add(this, &VrProteinApp::moleculePickerChangedCallback);
	moleculePicker->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	moleculePicker->manageChild();

	new GLMotif::Label("StyleLabel",settings,"Render style:");

	// Style picker radio box
	auto stylePicker = new GLMotif::RadioBox("StylePicker", settings, false);
	new GLMotif::ToggleButton("PointsBtn", stylePicker, "Points");
	new GLMotif::ToggleButton("SurfBtn", stylePicker, "Surf");
	stylePicker->getValueChangedCallbacks().add(this, &VrProteinApp::stylePickerChangedCallback);
	stylePicker->setSelectedToggle(1); // Surf default
	stylePicker->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	stylePicker->manageChild();

	auto colorBtn = new GLMotif::ToggleButton("ColorBtn", settings, "Use colors");
	colorBtn->setToggle(true); // UseColor default
	colorBtn->getValueChangedCallbacks().add(this, &VrProteinApp::colorToggleChangedCallback);

	settings->manageChild();

	return settingsDialog;
	}


/* Center display on currently loaded molecule */
void VrProteinApp::centerDisplayCallback(Misc::CallbackData* cbData) {
	std::cout << "Centering display." << std::endl;
	float x=0, y=0, z=0;
	if (drawMolecule) {
		drawMolecule->GetCenter(x, y, z);
	}
	Vrui::setNavigationTransformation(Vrui::Point(x, y, z), Vrui::Scalar(40));
}

void VrProteinApp::showSettingsDialogCallback(
		GLMotif::ToggleButton::ValueChangedCallbackData* cbData) {
	/* Hide or show settings dialog based on toggle button state: */
	if (cbData->set) {
		/* Pop up the settings dialog: */
		Vrui::popupPrimaryWidget(settingsDialog);
	} else
		Vrui::popdownPrimaryWidget(settingsDialog);
}

void VrProteinApp::settingsDialogCloseCallback(Misc::CallbackData* cbData) {
	showSettingsDialogToggle->setToggle(false);
}

/* Load a new molecule */
void VrProteinApp::moleculePickerChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData) {
	std::string name = cbData->newSelectedToggle->getString();
	std::cout << "Selected " << name << " from picker." << std::endl;
	LoadMolecule(name);
	/* Set the navigation transformation to show the entire scene: */
	centerDisplayCallback(nullptr);
}

/* Toggle draw style */
void VrProteinApp::stylePickerChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData) {
	std::string style = cbData->newSelectedToggle->getString();
	std::cout << "Selected style " << style << " from picker." << std::endl;
	if(style == "Points") {
		selectedStyle = DrawStyle::Points;
		drawMolecule->SetDrawStyle(DrawStyle::Points);
	}
	else if (style == "Surf") {
		selectedStyle = DrawStyle::Surf;
		drawMolecule->SetDrawStyle(DrawStyle::Surf);
	}
	else
		throw "Unknown style " + style;
}

/* Toggle use of color in molecules */
void VrProteinApp::colorToggleChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData) {
	selectedUseColor = cbData->set;
	drawMolecule->SetColorStyle(cbData->set);
}


/* Load a molecule from file */
void VrProteinApp::LoadMolecule(const std::string& fileName) {
	auto m = PDBImporter::ParsePDB(fileName);
	drawMolecule = unique_ptr<DrawMolecule>(new DrawMolecule(move(m)));

	drawMolecule->SetDrawStyle(selectedStyle);
	drawMolecule->SetColorStyle(selectedUseColor);
}



/* Create and execute an application object: */
int main(int argc, char* argv[])
{
	signal(SIGSEGV, handler);	// Generate debug info on crash.
	try {
		VrProteinApp app(argc, argv);
		app.run();
	} catch (std::runtime_error &err) {
		std::cerr << "Terminated program due to exception: " << err.what() << std::endl;
		return 1;
	}

	return 0;
}
