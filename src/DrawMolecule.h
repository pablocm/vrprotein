/*
 * DrawMolecule.h
 *
 *  Created on: Mar 16, 2014
 *      Author: pablocm
 */

#ifndef DRAWMOLECULE_H_
#define DRAWMOLECULE_H_

#include <memory>
#include <GL/GLColor.h>
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <Geometry/Vector.h>
#include <Vrui/Geometry.h>
#include "Molecule.h"

enum class DrawStyle { Points, Surf };

class DrawMolecule {
public:
	typedef GLVertex<void,0,GLfloat,4,GLfloat,GLfloat,3> Vertex; // Type for render vertices

	DrawMolecule(std::unique_ptr<Molecule> m);

	void Draw(GLContextData& contextData) const;
	void ComputeSurf();
	void GetCenter(float &x, float &y, float &z);
	void SetDrawStyle(DrawStyle style);
	void SetColorStyle(bool useColor);
private:
	std::unique_ptr<Molecule> molecule;
	std::vector<std::unique_ptr<Vertex>> vertices;
	bool surfComputed;
	DrawStyle style;
	bool useColor;

	void DrawPoints(GLContextData& contextData) const;
	void DrawSurf(GLContextData& contextData) const;
	std::unique_ptr<GLColor<GLfloat, 4>> AtomColor(char short_name) const;
};

#endif /* DRAWMOLECULE_H_ */
