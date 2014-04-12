/*
 * DrawMolecule.h
 *
 *  Created on: Mar 16, 2014
 *      Author: pablocm
 */

#ifndef DRAWMOLECULE_H_
#define DRAWMOLECULE_H_

#include <memory>
#include <GL/GLObject.h>
#include <GL/GLColor.h>
#include <GL/GLVertex.h>
#include "AffineSpace.h"
#include "Molecule.h"

/* Forward declarations: */
class GLContextData;

namespace VrProtein {

enum class DrawStyle {
	Points, Surf
};

class DrawMolecule: public GLObject {
public:
	/* Embedded classes: */
	typedef GLVertex<void, 0, GLfloat, 4, GLfloat, GLfloat, 3> Vertex;
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
	DrawMolecule(std::unique_ptr<Molecule> m);
	bool Intersects(const Ray& r) const;
	bool Intersects(const Point& p) const;
	bool Lock();
	void Unlock();
	ONTransform GetState() const;	// Returns position and orientation of molecule
	void SetState(const ONTransform& newState); // Sets state of molecule. Atom must be locked by caller.

	virtual void initContext(GLContextData& contextData) const;
	void glRenderAction(GLContextData& contextData) const;
	void ComputeSurf();
	void GetCenter(float &x, float &y, float &z);
	std::string GetName() const;
	void SetDrawStyle(DrawStyle style);
	void SetColorStyle(bool useColor);
private:
	std::unique_ptr<Molecule> molecule;
	std::vector<std::unique_ptr<Vertex>> vertices;
	Point position;
	Rotation orientation;
	bool surfComputed;
	DrawStyle style;
	bool useColor;
	bool locked;	// currently locked by a dragger

	void DrawPoints(GLContextData& contextData) const;
	void DrawSurf(GLContextData& contextData) const;
	std::unique_ptr<DrawMolecule::Color> AtomColor(char short_name) const;
};

}

#endif /* DRAWMOLECULE_H_ */
