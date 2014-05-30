/*
 * HudWidget.h
 *
 *  Created on: May 9, 2014
 *      Author: pablocm
 */

#ifndef HUDWIDGET_H_
#define HUDWIDGET_H_

#include <string>
#include <GLMotif/Draggable.h>
#include <GLMotif/Widget.h>
#include <Vrui/Geometry.h>
#include <Vrui/Vrui.h>
#include "AffineSpace.h"

/* Forward declarations: */
class GLLabel;
namespace GLMotif {
class WidgetManager;
}

namespace VrProtein {

class HudWidget: public GLMotif::Widget, public GLMotif::Draggable {
public:
	/* Embedded classes: */
	typedef GLColor<GLfloat, 4> Color;

	/* Methods: */
	HudWidget(const char* sName, GLMotif::WidgetManager* sManager, std::string sTitleString);
	virtual ~HudWidget();
	void setTitle(std::string newTitle);
	void setValue(Scalar newValue);
	void setOptions(bool useArcTan, Scalar minValue, Scalar maxValue, bool showMiddleLine,
			std::string valueFormat);

	/* Methods from GLMotif::Widget: */
	virtual const GLMotif::WidgetManager* getManager(void) const {
		return manager;
	}
	virtual GLMotif::WidgetManager* getManager(void) {
		return manager;
	}
	virtual GLMotif::Vector calcNaturalSize(void) const;
	virtual void resize(const GLMotif::Box& newExterior);
	virtual void draw(GLContextData& contextData) const;

private:
	/* Elements: */
	GLMotif::WidgetManager* manager; // Pointer to the widget manager
	std::string titleString;
	GLLabel* titleLabel; // Label to display the title
	Scalar currentValue; // Current display value
	GLLabel* valueLabel; // Label to display the current value
	bool showMiddleLine;
	bool useArctan;
	Scalar minValue;
	Scalar maxValue;
	std::string valueFormat;
};

} /* namespace VrProtein */
#endif /* HUDWIDGET_H_ */
