/*
 * MoleculeDragger.cpp
 *
 *  Created on: May 22, 2014
 *      Author: pablocm
 */

#include <Vrui/ToolManager.h>
#include "MoleculeDragger.h"
#include "DrawMolecule.h"
#include "VrProteinApp.h"

namespace VrProtein {

MoleculeDragger::MoleculeDragger(Vrui::DraggingTool* sTool, VrProteinApp* sApplication) :
			Vrui::DraggingToolAdapter(sTool),
			application(sApplication),
			dragging(false),
			moleculeIdx(-1) {
}

void MoleculeDragger::dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData) {
	/* Find the picked atom: */
	moleculeIdx = -1;
	if (cbData->rayBased) {
		std::cout << "Checking ray intersect" << std::endl;
		for (unsigned int i = 0; i < application->drawMolecules.size(); i++) {
			if (application->drawMolecules[i]->intersects(cbData->ray)) {
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
			if (application->drawMolecules[i]->intersects(point)) {
				moleculeIdx = i;
				break;
			}
		}
	}

	/* Try locking the atom: */
	if (moleculeIdx >= 0) {
		if (application->drawMolecules[moleculeIdx]->lock()) {
			std::cout << "Grabbed molecule." << std::endl;
			dragging = true;

			/* Calculate the initial transformation from the dragger to the dragged atom: */
			dragTransform = ONTransform(cbData->startTransformation.getTranslation(),
					cbData->startTransformation.getRotation());
			dragTransform.doInvert();
			dragTransform *= application->drawMolecules[moleculeIdx]->getState();
		}
		else
			std::cout << "Molecule is locked by another dragger." << std::endl;
	}
	else
		std::cout << "Nothing to grab at this location." << std::endl;
}

void MoleculeDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData) {
	if (dragging) {
		/* Apply the dragging transformation to the dragged atom: */
		ONTransform transform = ONTransform(cbData->currentTransformation.getTranslation(),
				cbData->currentTransformation.getRotation());
		transform *= dragTransform;
		application->drawMolecules[moleculeIdx]->setState(transform);
	}
}

void MoleculeDragger::dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData) {
	if (dragging) {
		std::cout << "Released molecule." << std::endl;
		/* Release the previously dragged atom: */
		application->drawMolecules[moleculeIdx]->unlock();
		moleculeIdx = -1;
		dragging = false;
	}
}

void MoleculeDragger::reset() {
	if (dragging) {
		std::cout << "Dragger reset." << std::endl;
		application->drawMolecules[moleculeIdx]->unlock();
		moleculeIdx = -1;
		dragging = false;
	}
}

} /* namespace VrProtein */
