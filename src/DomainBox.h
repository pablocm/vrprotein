/*
 * DomainBox.h
 *
 *  Created on: Apr 12, 2014
 *      Author: pablocm
 */

#ifndef DOMAINBOX_H_
#define DOMAINBOX_H_

#include <GL/GLColor.h>
#include <GL/GLObject.h>
#include "AffineSpace.h"

namespace VrProtein {

class DomainBox: public GLObject {
public:
	/* Embedded classes: */
	typedef GLColor<GLfloat, 4> Color;

	struct DataItem: public GLObject::DataItem {
	public:
		GLuint displayListId;

		/* Constructors and destructors: */
		DataItem(void) {
			/* Create the display list: */
			displayListId = glGenLists(1);
		}

		virtual ~DataItem(void) {
			/* Destroy the display list: */
			glDeleteLists(displayListId, 1);
		}
	};

	/* Public Methods: */
	DomainBox();
	virtual void initContext(GLContextData& contextData) const;
	void glRenderAction(GLContextData& contextData) const;

private:
	/* Private fields */
	Point min;
	Point max;
	Color lineColor;
	Color originColor;
};

}

#endif /* DOMAINBOX_H_ */
