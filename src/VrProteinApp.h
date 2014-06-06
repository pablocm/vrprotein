/*
 * VrProteinApp.h
 *
 *  Created on: May 22, 2014
 *      Author: pablocm
 */

#ifndef VRPROTEINAPP_H_
#define VRPROTEINAPP_H_

#include <memory>
#include <unordered_map>
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
namespace Misc {
class File;
}
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
	void setForceAttenuation(Scalar factor);
	void setColorStyle(ColorStyle newStyle, bool refreshUI = true);
	void setDrawStyle(DrawStyle newStyle, bool refreshUI = true);
	void debug();
	void setupExperiment(int experimentId);
	void moveToPocket(int pocketId);	// Move to best current solution of pocket id
	void toggleClosestVisibility();	// Toggle visibility for solution at closest pocket
	void toggleAllVisibility(); // Toggle visibility for ALL solutions
	void saveSolution();
	void refreshSettingsDialog(bool rebuildMoleculeSelector = true);

private:
	friend class MoleculeDragger;

	/* Fields: */
	double lastFrameTime;
	Simulator simulator;
	DomainBox domainBox;
	std::vector<std::unique_ptr<DrawMolecule>> drawMolecules;
	std::unordered_map<int, std::unique_ptr<DrawMolecule>> bestDrawMolecules;
	DrawStyle selectedStyle;
	ColorStyle selectedColorStyle;
	int selectedMoleculeIdx;
	// statistics
	bool frameSkip;
	bool isSimulating;
	bool isCalculatingForces;
	Scalar forceAttenuation;	// 1.0 = Normal, 0.0 = Forces do not affect molecule.
	Simulator::SimResult latestSimResult;
	std::unordered_map<int, Simulator::SimResult> bestSimResults;
	// Experiment
	std::unique_ptr<Misc::File> experimentFile;

	/* Methods: */
	std::unique_ptr<DrawMolecule> CreateMolecule(const std::string& fileName) const;
	int IndexOfMolecule(const std::string& moleculeName) const;
	std::vector<std::string> GetDropdownItemStrings() const;
	//Helper
	std::string ONTransformToString(const ONTransform& transform) const;
	// Tool items
	std::vector<std::unique_ptr<MoleculeDragger>> moleculeDraggers;
	// UI Items
	GLMotif::PopupMenu* mainMenuPopup; // The program's main menu
	GLMotif::ToggleButton* showSettingsDialogToggle;
	GLMotif::ToggleButton* showStatisticsDialogToggle;
	GLMotif::ToggleButton* showHudWidgetToggle;
	GLMotif::PopupWindow* settingsDialog; // The settings dialog
	GLMotif::DropdownBox* moleculeSelector;	// dropdown for molecule selector
	GLMotif::RadioBox* stylePicker; // Radio box for draw style
	GLMotif::RadioBox* colorStylePicker; // Radio box for color style
	GLMotif::PopupWindow* statisticsDialog; // The statistics dialog
	std::unique_ptr<HudWidget> overlapWidget;
	std::unique_ptr<HudWidget> distanceWidget;
	GLMotif::ToggleButton* simulateBtn;		// Toggle for isSimulating
	GLMotif::ToggleButton* calculateForcesBtn; // Toggle for isCalculatingForces
	GLMotif::TextField* heuristicTextField;	// Current value for heuristic
	GLMotif::TextField* overlappingTextField;	// Current value for overlapping
	GLMotif::TextField* closestPocketTextField; // Current value for closest pocket
	GLMotif::TextField* meanDistanceTextField;	// Current value for mean distance to pocket
	GLMotif::TextField* frameRateTextField;		// Current value for frame rate
	// UI Constructors
	void createMainMenu(void);
	void createSettingsDialog(void);
	void createStatisticsDialog(void);
	// UI Callbacks
	void moleculeSelectorChangedCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void moleculeLoaderChangedCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
};

}

#endif /* VRPROTEINAPP_H_ */
