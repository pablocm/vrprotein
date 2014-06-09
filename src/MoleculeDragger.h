/*
 * MoleculeDragger.h
 *
 *  Created on: May 22, 2014
 *      Author: pablocm
 */

#ifndef MOLECULEDRAGGER_H_
#define MOLECULEDRAGGER_H_

#include <Vrui/DraggingToolAdapter.h>
#include "AffineSpace.h"

/* Forward Declarations: */
namespace VrProtein {
class VrProteinApp;
}

namespace VrProtein {

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
	void reset();	// halt dragging right away
};

}

#endif /* MOLECULEDRAGGER_H_ */
