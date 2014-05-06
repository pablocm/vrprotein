/*
 * DomainBox.cpp
 *
 *  Created on: Apr 12, 2014
 *      Author: pablocm
 */


#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include "DomainBox.h"

namespace VrProtein {

DomainBox::DomainBox() :
			min(Point(-40, -40, -40)),
			max(Point(40, 40, 40)),
			lineColor(Color(0, 1, 0)),
			originColor(Color(1, 1, 1)) {
}

void DomainBox::initContext(GLContextData& contextData) const {
	DataItem* dataItem = new DataItem;
	contextData.addDataItem(this, dataItem);

	/* Create the domain box display list: */
	glNewList(dataItem->displayListId, GL_COMPILE);
	{
		/* Set up OpenGL state: */
		glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
		{
			glDisable(GL_LIGHTING);
			glLineWidth(2.0f);
			/* Grid's domain box: */
			glColor(lineColor);
			glBegin(GL_LINE_STRIP);
			{
				glVertex(min[0], min[1], min[2]);
				glVertex(max[0], min[1], min[2]);
				glVertex(max[0], max[1], min[2]);
				glVertex(min[0], max[1], min[2]);
				glVertex(min[0], min[1], min[2]);
				glVertex(min[0], min[1], max[2]);
				glVertex(max[0], min[1], max[2]);
				glVertex(max[0], max[1], max[2]);
				glVertex(min[0], max[1], max[2]);
				glVertex(min[0], min[1], max[2]);
			}
			glEnd();
			glBegin(GL_LINES);
			{
				glVertex(max[0], min[1], min[2]);
				glVertex(max[0], min[1], max[2]);
				glVertex(max[0], max[1], min[2]);
				glVertex(max[0], max[1], max[2]);
				glVertex(min[0], max[1], min[2]);
				glVertex(min[0], max[1], max[2]);
			}
			glEnd();
			/* Origin lines */
			glColor(originColor);
			glBegin(GL_LINES);
			{
				glVertex(-10, 0, 0);
				glVertex(10, 0, 0);
				glVertex(0, -10, 0);
				glVertex(0, 10, 0);
				glVertex(0, 0, -10);
				glVertex(0, 0, 10);
			}
			glEnd();
		}
		/* Reset OpenGL state: */
		glPopAttrib();
	}
	glEndList();
}

void DomainBox::glRenderAction(GLContextData& contextData) const {
	/* Get the OpenGL-dependent application data from the GLContextData object: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	/* Call the display list */
	glCallList(dataItem->displayListId);
}

}
