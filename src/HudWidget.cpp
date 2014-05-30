/*
 * HudWidget.cpp
 *
 *  Created on: May 9, 2014
 *      Author: pablocm
 */

#include "HudWidget.h"
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLFont.h>
#include <GL/GLLabel.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>

namespace VrProtein {

HudWidget::HudWidget(const char* sName, GLMotif::WidgetManager* sManager, std::string sTitleString) :
			GLMotif::Widget(sName, 0, false),
			manager(sManager),
			titleString(sTitleString) {
	/* Set widget parameters: */
	setBorderWidth(0.0f);
	setBorderType(GLMotif::Widget::PLAIN);

	/* Set default background and foreground colors: */
	Color bgColor = Vrui::getBackgroundColor();
	bgColor[3] = 0.0f;
	Color fgColor;
	for (int i = 0; i < 3; ++i)
		fgColor[i] = 1.0f - bgColor[i];
	fgColor[3] = 1.0f;
	setBorderColor(bgColor);
	setBackgroundColor(bgColor);
	setForegroundColor(fgColor);

	/* Create the initial title label */
	titleLabel = new GLLabel(titleString.c_str(), *Vrui::getUiFont());
	titleLabel->setBackground(bgColor);
	titleLabel->setForeground(fgColor);

	/* Create the initial value label */
	valueLabel = new GLLabel("---", *Vrui::getUiFont());
	valueLabel->setBackground(bgColor);
	valueLabel->setForeground(fgColor);

	/* Resize the widget: */
	GLMotif::Vector newSize = calcNaturalSize();
	GLMotif::Vector newOrigin = GLMotif::Vector(0.0f, 0.0f, 0.0f);
	newOrigin[0] = -newSize[0] * 0.5f;
	resize(GLMotif::Box(newOrigin, newSize));
}

HudWidget::~HudWidget(void) {
	/* Pop down the widget: */
	manager->popdownWidget(this);

	/* Delete the length and scale labels: */
	delete valueLabel;

	/* Unmanage the widget itself: */
	manager->unmanageWidget(this);
}

void HudWidget::setTitle(std::string newTitle) {
	if (titleString != newTitle) {
		titleString = newTitle;
		titleLabel->setString(newTitle.c_str());
	}
}

void HudWidget::setValue(Scalar newValue) {
	if (currentValue != newValue) {
		currentValue = newValue;
		char buffer[32];
		snprintf(buffer, 32, "%4.0f", newValue);
		valueLabel->setString(buffer);
	}
}

GLMotif::Vector HudWidget::calcNaturalSize() const {
	GLMotif::Vector result(0, Vrui::getUiFont()->getTextHeight() * 3.0f, 0.0f);

	if (titleLabel != nullptr) {
		/* Adjust for the label size: */
		const GLLabel::Box::Vector& labelSize = titleLabel->getLabelSize();
		if (result[0] < labelSize[0])
			result[0] = labelSize[0];
	}
	if (valueLabel != nullptr) {
		/* Adjust for the label size: */
		const GLLabel::Box::Vector& labelSize = valueLabel->getLabelSize();
		if (result[0] < labelSize[0])
			result[0] = labelSize[0];
	}

	/* Calculate the scale bar's current size: */
	return calcExteriorSize(result);
}

void HudWidget::resize(const GLMotif::Box& newExterior) {
	/* Resize the parent class widget: */
	GLMotif::Widget::resize(newExterior);

	if (titleLabel != nullptr) {
		/* Reposition the label: */
		const GLLabel::Box::Vector& labelSize = titleLabel->getLabelSize();
		GLLabel::Box::Vector labelPos;
		labelPos[0] = getInterior().origin[0] + (getInterior().size[0] - labelSize[0]) * 1.5f;
		labelPos[1] = getInterior().origin[1] + getInterior().size[1] * 0.5f + labelSize[1];
		labelPos[2] = 0.0f;
		titleLabel->setOrigin(labelPos);
	}
	if (valueLabel != nullptr) {
		/* Reposition the label: */
		const GLLabel::Box::Vector& labelSize = valueLabel->getLabelSize();
		GLLabel::Box::Vector labelPos;
		labelPos[0] = getInterior().origin[0] + (getInterior().size[0] - labelSize[0]) * 0.5f;
		labelPos[1] = getInterior().origin[1] + getInterior().size[1] * 0.5f - labelSize[1] * 1.5f;
		labelPos[2] = 0.0f;
		valueLabel->setOrigin(labelPos);
	}
}

void HudWidget::draw(GLContextData& contextData) const {
	/* Save and set OpenGL state: */
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);

	const float currentPhysLength = titleLabel->getLabelSize()[0];
	/* Calculate the scale bar layout: */
	GLfloat x0 = getInterior().origin[0]
			+ (getInterior().size[0] - GLfloat(currentPhysLength)) * 0.5f;
	GLfloat x1 = x0 + GLfloat(currentPhysLength);
	const GLLabel::Box::Vector& labelSize = titleLabel->getLabelSize();
	GLfloat y0 = getInterior().origin[1] + (getInterior().size[1] - labelSize[1] * 2.0f) * 0.5f;
	GLfloat y1 = y0 + labelSize[1];
	GLfloat y2 = y1 + labelSize[1];

	// Scale currentValue between -1 and 1 using non-linear arc tangent
	GLfloat valueScaled = Math::atan(GLfloat(currentValue) / 50) * 2 / Math::Constants<GLfloat>::pi;
	GLfloat xm = (x0 + x1) / 2 + (x1 - x0) / 2 * valueScaled;

	/* Draw the measure bar: */
	glLineWidth(5.0f);	// shadow for horizontal line
	glBegin(GL_LINES);
	glColor(getBackgroundColor());
	glVertex2f(x0, y1);
	glVertex2f(x1, y1);
	glVertex2f(xm, y0);	// shadow for currentValue line
	glVertex2f(xm, y2);
	glEnd();

	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glVertex2f(x0, y0); // shadow for both extreme vertical lines
	glVertex2f(x0, y2);
	glVertex2f(x1, y0);
	glVertex2f(x1, y2);
	glVertex2f((x0 + x1) / 2, (y0 + y1) / 2);	// shadow for optimal value line
	glVertex2f((x0 + x1) / 2, (y1 + y2) / 2);

	glColor(getForegroundColor()); // white horizontal line
	glVertex2f(x0, y1);
	glVertex2f(x1, y1);
	glEnd();

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex2f(x0, y0); // white extreme vertical lines
	glVertex2f(x0, y2);
	glVertex2f(x1, y0);
	glVertex2f(x1, y2);
	glVertex2f((x0 + x1) / 2, (y0 + y1) / 2);	// white optimal value line
	glVertex2f((x0 + x1) / 2, (y1 + y2) / 2);
	glEnd();

	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glColor(Color(0.8f, 0.2f, 0.1f));
	glVertex2f(xm, y0);	// red currentValue line
	glVertex2f(xm, y2);
	glEnd();

	/* Install a temporary deferred renderer: */
	{
		GLLabel::DeferredRenderer dr(contextData);

		/* Draw the labels: */
		// glEnable(GL_ALPHA_TEST);
		// glAlphaFunc(GL_GREATER,0.0f);
		titleLabel->draw(contextData);
		valueLabel->draw(contextData);
	}

	/* Restore OpenGL state: */
	glPopAttrib();
}

} /* namespace VrProtein */
