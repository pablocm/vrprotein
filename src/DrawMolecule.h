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

	DrawStyle style;

	DrawMolecule(std::unique_ptr<Molecule> m);

	void Draw(GLContextData& contextData) const;
	void ComputeSurf();
private:
	std::unique_ptr<Molecule> molecule;
	//std::vector<Vrui::Vector> vertices;
	//std::vector<Vrui::Vector> normals;
	//std::vector<GLColor<GLfloat, 4>> colors;
	std::vector<std::unique_ptr<Vertex>> vertices;
	bool surfComputed;

	void DrawPoints(GLContextData& contextData) const;
	void DrawSurf(GLContextData& contextData) const;
	std::unique_ptr<GLColor<GLfloat, 4>> AtomColor(char short_name) const;
};

#endif /* DRAWMOLECULE_H_ */
