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
enum class ColorStyle;
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
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);

	/* Methods: */
	void centerDisplay();
	void toggleSimulation(bool simulate, bool refreshUI = true);
	void toggleForces(bool calculateForces, bool refreshUI = true);
	void setColorStyle(ColorStyle newStyle, bool refreshUI = true);
	void setDrawStyle(DrawStyle newStyle, bool refreshUI = true);

private:
	friend class MoleculeDragger;

	/* Fields: */
	double lastFrameTime;
	Simulator simulator;
	DomainBox domainBox;
	std::vector<std::unique_ptr<DrawMolecule>> drawMolecules;
	DrawStyle selectedStyle;
	ColorStyle selectedColorStyle;
	int selectedMoleculeIdx;
	// statistics
	bool isSimulating;
	bool isCalculatingForces;
	Simulator::SimResult simResult;

	/* Methods: */
	std::unique_ptr<DrawMolecule> LoadMolecule(const std::string& fileName);
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
	GLMotif::RadioBox* stylePicker; // Radio box for draw style
	GLMotif::RadioBox* colorStylePicker; // Radio box for color style
	GLMotif::PopupWindow* statisticsDialog; // The statistics dialog
	HudWidget* hudWidget;
	GLMotif::ToggleButton* simulateBtn;		// Toggle for isSimulating
	GLMotif::ToggleButton* calculateForcesBtn; // Toggle for isCalculatingForces
	GLMotif::TextField* heuristicTextField;	// Current value for heuristic
	GLMotif::TextField* overlappingTextField;	// Current value for overlapping
	// UI Constructors
	GLMotif::PopupMenu* createMainMenu(void);
	GLMotif::PopupWindow* createSettingsDialog(void);
	GLMotif::PopupWindow* createStatisticsDialog(void);
	// UI Callbacks
	void moleculeSelectorChangedCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void moleculeLoaderChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
};

}

#endif /* VRPROTEINAPP_H_ */