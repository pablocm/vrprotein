/*
 * SimulationToggleTool.h
 *
 *  Created on: May 23, 2014
 *      Author: pablocm
 */

#ifndef SIMULATIONTOGGLETOOL_H_
#define SIMULATIONTOGGLETOOL_H_

#include <Vrui/GenericToolFactory.h>
#include "VrProteinApp.h"

namespace VrProtein {

class SimulationControlTool: public Vrui::Tool, public Vrui::Application::Tool<VrProteinApp> {
public:
	/* Methods: */
	SimulationControlTool(const Vrui::ToolFactory* factory,
			const Vrui::ToolInputAssignment& inputAssignment);
	static void registerTool(Vrui::ToolManager& toolManager); // Register tool class in Vrui

	/* Methods from Vrui::Tool: */
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void initialize(void);
	virtual void buttonCallback(int buttonSlotIndex, Vrui::InputDevice::ButtonCallbackData* cbData);

private:
	/* Elements: */
	typedef Vrui::GenericToolFactory<SimulationControlTool> ToolFactory; // Use generic factory class
	friend class Vrui::GenericToolFactory<SimulationControlTool>;
	static ToolFactory* factory; // Pointer to the factory object for this class
};

}

#endif /* SIMULATIONTOGGLETOOL_H_ */
