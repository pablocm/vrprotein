/*
 * SimulationToggleTool.cpp
 *
 *  Created on: May 23, 2014
 *      Author: pablocm
 */

#include <Vrui/ToolManager.h>
#include "SimulationControlTool.h"

namespace VrProtein {

SimulationControlTool::ToolFactory* SimulationControlTool::factory = nullptr;

SimulationControlTool::SimulationControlTool(const Vrui::ToolFactory* factory,
		const Vrui::ToolInputAssignment& inputAssignment) :
			Vrui::Tool(factory, inputAssignment) {
}

void SimulationControlTool::registerTool(Vrui::ToolManager& toolManager) {
	/* Create a factory object for the custom tool class: */
	factory = new ToolFactory("SimControlTool", "Simulation Control Tool", nullptr, toolManager);

	/* Set the custom tool class' input layout: */
	factory->setNumButtons(1, true); // Needs one button and takes optional buttons
	factory->setButtonFunction(0, "Toggle Simulation");
	factory->setButtonFunction(1, "Toggle Forces");

	/* Register the custom tool class with the Vrui tool manager: */
	toolManager.addClass(factory, Vrui::ToolManager::defaultToolFactoryDestructor);
}

const Vrui::ToolFactory* SimulationControlTool::getFactory(void) const {
	return factory;
}

void SimulationControlTool::initialize() {
	std::cout << "SimControlTool initialized." << std::endl;
}

void SimulationControlTool::buttonCallback(int buttonSlotIndex,
		Vrui::InputDevice::ButtonCallbackData* cbData) {
	if (cbData->newButtonState) {
		// Button has just been pressed
		if (buttonSlotIndex == 0)
			application->toggleSimulation(true);
		else if (buttonSlotIndex == 1)
			application->toggleForces(true);
	}
	else {
		// Button has just been released
		if (buttonSlotIndex == 0)
			application->toggleSimulation(false);
		else if (buttonSlotIndex == 1)
			application->toggleForces(false);
	}
}

}
