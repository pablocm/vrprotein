/*
 * VrProteinApp.h
 *
 *  Created on: May 22, 2014
 *      Author: pablocm
 */

#ifndef VRPROTEINAPP_H_
#define VRPROTEINAPP_H_

#include <memory>
#include <GLMotif/DropdownBox.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/TextField.h>
#include <GLMotif/ToggleButton.h>
#include <Vrui/Application.h>
#include <Vrui/Vrui.h>
#include "AffineSpace.h"
#include "DomainBox.h"
#include "Simulator.h"

/* Forward declarations: */
namespace VrProtein {
class DrawMolecule;
enum class DrawStyle;
class HudWidget;
class MoleculeDragger;
}

namespace VrProtein {

class VrProteinApp: public Vrui::Application {
public:
	VrProteinApp(int& argc, char**& argv);

	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void frame();

private:
	friend class MoleculeDragger;

	// Private fields
	Simulator simulator;
	DomainBox domainBox;
	std::vector<std::unique_ptr<DrawMolecule>> drawMolecules;
	DrawStyle selectedStyle;
	bool selectedUseColor;
	int selectedMoleculeIdx;
	// statistics
	bool isSimulating;
	bool isCalculatingForces;
	Simulator::SimResult simResult;
	// Private methods
	std::unique_ptr<DrawMolecule> LoadMolecule(const std::string& fileName);
	void SetDrawStyle(DrawStyle style);
	int IndexOfMolecule(const std::string& moleculeName) const;
	std::vector<std::string> GetDropdownItemStrings() const;
	// Tool items
	std::vector<std::unique_ptr<MoleculeDragger>> moleculeDraggers;
	// UI Items
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	GLMotif::ToggleButton* showSettingsDialogToggle;
	GLMotif::ToggleButton* showStatisticsDialogToggle;
	GLMotif::ToggleButton* showHudWidgetToggle;
	GLMotif::PopupWindow* settingsDialog; // The settings dialog
	GLMotif::DropdownBox* moleculeSelector;	// dropdown for molecule selector
	GLMotif::PopupWindow* statisticsDialog; // The statistics dialog
	HudWidget* hudWidget;
	GLMotif::TextField* heuristicTextField;	// Current value for heuristic
	GLMotif::TextField* overlappingTextField;	// Current value for overlapping
	// UI Constructors
	GLMotif::PopupMenu* createMainMenu(void);
	GLMotif::PopupWindow* createSettingsDialog(void);
	GLMotif::PopupWindow* createStatisticsDialog(void);
	// UI Callbacks
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void showSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showStatisticsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showHudWidgetCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void settingsDialogCloseCallback(Misc::CallbackData* cbData);
	void statisticsDialogCloseCallback(Misc::CallbackData* cbData);
	void moleculeSelectorChangedCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void moleculeLoaderChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void stylePickerChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void colorToggleChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void simulateToggleChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void calculateForcesToggleChangedCallback(
			GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	// Tool Callbacks
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
};

}

#endif /* VRPROTEINAPP_H_ */
