/*
 * ExperimentControlTool.h
 *
 *  Created on: May 23, 2014
 *      Author: pablocm
 */

#ifndef EXPERIMENTCONTROLTOOL_H_
#define EXPERIMENTCONTROLTOOL_H_

#include <Vrui/GenericToolFactory.h>
#include "VrProteinApp.h"

namespace VrProtein {

class ExperimentControlTool: public Vrui::Tool, public Vrui::Application::Tool<VrProteinApp> {
public:
	/* Methods: */
	ExperimentControlTool(const Vrui::ToolFactory* factory,
			const Vrui::ToolInputAssignment& inputAssignment);
	static void registerTool(Vrui::ToolManager& toolManager); // Register tool class in Vrui

	/* Methods from Vrui::Tool: */
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void initialize(void);
	virtual void buttonCallback(int buttonSlotIndex, Vrui::InputDevice::ButtonCallbackData* cbData);

private:
	/* Elements: */
	typedef Vrui::GenericToolFactory<ExperimentControlTool> ToolFactory; // Use generic factory class
	friend class Vrui::GenericToolFactory<ExperimentControlTool>;
	static ToolFactory* factory; // Pointer to the factory object for this class
};

}

#endif /* EXPERIMENTCONTROLTOOL_H_ */
