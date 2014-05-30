/*
 * ExperimentControlTool.cpp
 *
 *  Created on: May 23, 2014
 *      Author: pablocm
 */

#include <Vrui/ToolManager.h>
#include "ExperimentControlTool.h"

namespace VrProtein {

ExperimentControlTool::ToolFactory* ExperimentControlTool::factory = nullptr;

ExperimentControlTool::ExperimentControlTool(const Vrui::ToolFactory* factory,
		const Vrui::ToolInputAssignment& inputAssignment) :
			Vrui::Tool(factory, inputAssignment) {
}

void ExperimentControlTool::registerTool(Vrui::ToolManager& toolManager) {
	/* Create a factory object for the custom tool class: */
	factory = new ToolFactory("ExperimentControlTool", "Experiment Control Tool", nullptr,
			toolManager);

	/* Set the custom tool class' input layout: */
	factory->setNumButtons(7, false);
	factory->setButtonFunction(0, "Save solution");
	factory->setButtonFunction(1, "Setup exp 1");
	factory->setButtonFunction(2, "Setup exp 2");
	factory->setButtonFunction(3, "Setup exp 3");
	factory->setButtonFunction(4, "Setup exp 4");
	factory->setButtonFunction(5, "Setup exp 5");
	factory->setButtonFunction(6, "Debug");

	/* Register the custom tool class with the Vrui tool manager: */
	toolManager.addClass(factory, Vrui::ToolManager::defaultToolFactoryDestructor);
}

const Vrui::ToolFactory* ExperimentControlTool::getFactory(void) const {
	return factory;
}

void ExperimentControlTool::initialize() {
	std::cout << "ExperimentControlTool initialized." << std::endl;
}

void ExperimentControlTool::buttonCallback(int buttonSlotIndex,
		Vrui::InputDevice::ButtonCallbackData* cbData) {
	if (cbData->newButtonState) {
		// Button has just been pressed
		if (buttonSlotIndex == 0)
			application->saveSolution();
		else if (buttonSlotIndex == 6)
			application->debug();
		else
			application->setupExperiment(buttonSlotIndex);
	}
	else {
		// Button has just been released
	}
}

}
