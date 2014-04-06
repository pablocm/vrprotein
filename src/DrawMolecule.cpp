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
#include <GL/GLTransformationWrappers.h>
#include <Geometry/Sphere.h>
#include <Geometry/Vector.h>
#include <Vrui/Geometry.h>
#include "DrawMolecule.h"
#include "Molecule.h"

using namespace std;

DrawMolecule::DrawMolecule(unique_ptr<Molecule> m) {
	molecule = move(m);
	style = DrawStyle::Points;
	surfComputed = false;
	useColor = true;
	locked = false;
	position = Vrui::Point::origin;
	orientation = Vrui::Rotation::identity;
}


bool DrawMolecule::Intersects(const Vrui::Ray& r) const {
	Geometry::Sphere<Vrui::Scalar, 3> sphere(Vrui::Point::origin, 0);  // Test sphere
	for (auto& atom : molecule->GetAtoms()) {
		sphere.setCenter(
				Vrui::Point(atom->x + position[0], atom->y + position[1], atom->z + position[2]));
		sphere.setRadius(atom->radius * 1.5);

		auto hitResult = sphere.intersectRay(r);

		if (hitResult.isValid())
			return true;
	}
	return false;
}

bool DrawMolecule::Intersects(const Vrui::Point& p) const {
	std::cout << "Checking intersect at " << p[0] << ", " << p[1] << ", " << p[2] << std::endl;
	auto transform = GetState();
	for (auto& atom : molecule->GetAtoms()) {
		auto position = Vrui::Point(atom->x, atom->y, atom->z);
		position = transform.transform(position);

		auto dist2 = Geometry::sqrDist(p, position);
		if (dist2 <= Math::sqr(atom->radius * 1.5))
			return true;
	}
	return false;
}

bool DrawMolecule::Lock() {
	if (!locked)
		return locked = true;
	return false;
}

void DrawMolecule::Unlock() {
	locked = false;
}

Vrui::ONTransform DrawMolecule::GetState() const {
	return Vrui::ONTransform(position - Vrui::Point::origin, orientation);
}

void DrawMolecule::SetState(const Vrui::ONTransform& newState) {
	position = newState.getOrigin();
	orientation = newState.getRotation();
}


void DrawMolecule::Draw(GLContextData& contextData) const {
	glPushMatrix();
	glMultMatrix(GetState());

	switch (style) {
	case DrawStyle::Points:
		DrawPoints(contextData); break;
	case DrawStyle::Surf:
		DrawSurf(contextData); break;
	}

	glPopMatrix();
}


/**
 * Dibujar molecula usando esferas.
 */
void DrawMolecule::DrawPoints(GLContextData& contextData) const {
	//glPointSize(2.0f);

	//glBegin(GL_POINTS);
	for (auto& a : molecule->GetAtoms()) {
		if (useColor) {
			auto color = AtomColor(a->short_name);
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, *color);
		} else {
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT,
					GLColor<GLfloat, 4>(0.9f, 0.9f, 0.9f));
		}

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

	if (useColor) {
		for (auto& v : vertices) {
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, v->color);
			glVertex(*v);
		}
	}
	else {
		glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(0.9f, 0.9f, 0.9f));
		for (auto& v : vertices) {
			glVertex(*v);
		}
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


void DrawMolecule::GetCenter(float &x, float &y, float &z) {
	molecule->GetCenter(x, y, z);
}

void DrawMolecule::SetColorStyle(bool useColor) {
	this->useColor = useColor;
}

void DrawMolecule::SetDrawStyle(DrawStyle style) {
	this->style = style;
	if (style == DrawStyle::Surf) {
		ComputeSurf();
	}
}
