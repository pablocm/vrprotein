/*
 * DrawMolecule.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: pablocm
 */

#include <memory>
#include <fstream>
#include <iostream>
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <Geometry/Vector.h>
#include <Vrui/Geometry.h>
#include "DrawMolecule.h"
#include "Molecule.h"

using namespace std;

DrawMolecule::DrawMolecule(unique_ptr<Molecule> m) {
	molecule = move(m);
	style = DrawStyle::Points;
	surfComputed = false;
}


void DrawMolecule::Draw(GLContextData& contextData) const {
	switch (style) {
	case DrawStyle::Points:
		DrawPoints(contextData); break;
	case DrawStyle::Surf:
		DrawSurf(contextData); break;
	}
}


/**
 * Dibujar molecula usando esferas.
 */
void DrawMolecule::DrawPoints(GLContextData& contextData) const {
	glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(0.5f, 0.5f, 1.0f));
	glDrawSphereIcosahedron(0.1f, 3);

	glPointSize(2.0f);

	//glBegin(GL_POINTS);
	for (auto& a : molecule->GetAtoms()) {
		auto color = AtomColor(a->short_name);
		glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, *color);

		//glVertex3f(a->x, a->y, a->z);

		glPushMatrix();
		glTranslated(a->x, a->y, a->z);
		glDrawSphereIcosahedron(a->radius/2, 1);
		glPopMatrix();
	}
	//glEnd();
}


/**
 * Dibujar usando superficie.
 */
void DrawMolecule::DrawSurf(GLContextData& contextData) const {
	if (!surfComputed)
		throw std::runtime_error("need to compute surf before draw");

	glBegin(GL_TRIANGLES);

	for(auto& v : vertices) {
		glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, v->color);
		glVertex(*v);
	}

	glEnd();
}


void DrawMolecule::ComputeSurf() {
	if (surfComputed)
		return;
	// Ver si existe un archivo .tri ya generado por SURF
	cout << "Trying to open " << molecule->source_filename << ".tri ...";
	ifstream infile;
	infile.open(molecule->source_filename + ".tri");
	if (!infile.fail()) {
		cout << "success." << endl;

		string line;
		while (getline(infile, line)) {
			int atom_id = stoi(line);
			// leer 3 vertices del triangulo
			for(int i = 0; i < 3; i++) {
				if(!getline(infile, line))
					throw "Error leyendo .tri";
				float x, y, z, nx, ny, nz;
				if (6 != sscanf(line.c_str(), "%f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz))
					throw "Error parseando .tri";

				auto vertex = unique_ptr<Vertex>(new Vertex());
				vertex->color = *AtomColor(molecule->GetAtoms()[atom_id]->short_name).release();
				//vertex->color = GLColor<GLfloat, 4>(0.9f, 0.9f, 0.9f);
				vertex->normal = Vertex::Normal(nx, ny, nz);
				vertex->position = Vertex::Position(x ,y ,z);

				vertices.push_back(move(vertex));
			}
		}
		infile.close();

		cout << "Surf computed. Loaded " << (vertices.size()/3) << " triangles." << endl;
	}
	else {
		cout << "not found." << endl;
		throw "TODO: Hacer surf";
	}
	surfComputed = true;
}


unique_ptr<GLColor<GLfloat, 4>> DrawMolecule::AtomColor(char short_name) const {
	switch(short_name) {
	case 'H': return unique_ptr<GLColor<GLfloat, 4>>(new GLColor<GLfloat, 4>(1.0f, 0.0f, 0.0f)); // rojo
	case 'C': return unique_ptr<GLColor<GLfloat, 4>>(new GLColor<GLfloat, 4>(0.0f, 1.0f, 0.0f)); // verde
	case 'N': return unique_ptr<GLColor<GLfloat, 4>>(new GLColor<GLfloat, 4>(1.0f, 1.0f, 0.0f)); // amarillo
	case 'O': return unique_ptr<GLColor<GLfloat, 4>>(new GLColor<GLfloat, 4>(0.2f, 0.2f, 1.0f)); // azul
	}
	return unique_ptr<GLColor<GLfloat, 4>>(new GLColor<GLfloat, 4>(0.9f, 0.9f, 0.9f)); // blanco
}

/**
 * Ver:
 * DrawMolItemSurface.C:42 (draw_surface)
 * Surf.C:35 (compute)
 *
 * Para parsear output:
 * http://www.cplusplus.com/reference/cstdio/fscanf/
 */
