/*
 * DrawMolecule.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: pablocm
 */

#include <fstream>
#include <iostream>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <Geometry/Sphere.h>
#include "DrawMolecule.h"
#include "Molecule.h"

using namespace std;

namespace VrProtein {

/****************************************
Methods of class DrawMolecule::DataItem:
****************************************/

DrawMolecule::DataItem::DataItem() :
			hasVertexBufferObjectExtension(GLARBVertexBufferObject::isSupported()),
			vertexDataVersion(0) {
	// this is not used, just for future reference
	if (hasVertexBufferObjectExtension) {
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();

		/* Create the vertex and index buffer objects: */
		glGenBuffersARB(6, faceVertexBufferObjectIDs);
		glGenBuffersARB(6, faceIndexBufferObjectIDs);
	}

	// Create display list for storing molecule rendering
	displayListId = glGenLists(1);
	displayListDrawStyle = DrawStyle::None;
	displayListColorStyle = ColorStyle::None;
}

DrawMolecule::DataItem::~DataItem(void) {
	if (hasVertexBufferObjectExtension) {
		/* Destroy the vertex and index buffer objects: */
		glDeleteBuffersARB(6, faceVertexBufferObjectIDs);
		glDeleteBuffersARB(6, faceIndexBufferObjectIDs);
	}

	// Destroy the display list
	glDeleteLists(displayListId, 1);
}

/*****************************
Methods of class DrawMolecule:
*****************************/

DrawMolecule::DrawMolecule(unique_ptr<Molecule> m) {
	molecule = move(m);
	style = DrawStyle::Points;
	surfComputed = false;
	pocketsComputed = false;
	colorStyle = ColorStyle::CPK;
	locked = false;
	velocity = Vector::zero;
	position = Point::origin;
	orientation = Rotation::identity;
}

bool DrawMolecule::Intersects(const Ray& r) const {
	auto transform = GetState();
	Geometry::Sphere<Scalar, 3> sphere(Point::origin, 0);  // Test sphere
	for (const auto& atom : molecule->GetAtoms()) {
		auto position = transform.transform(atom->position);
		sphere.setCenter(position);
		sphere.setRadius(atom->radius * 1.5);

		auto hitResult = sphere.intersectRay(r);

		if (hitResult.isValid())
			return true;
	}
	return false;
}

bool DrawMolecule::Intersects(const Point& p) const {
	auto transform = GetState();
	for (const auto& atom : molecule->GetAtoms()) {
		auto position = transform.transform(atom->position);

		auto dist2 = Geometry::sqrDist(p, position);
		if (dist2 <= Math::sqr(atom->radius * 1.5))
			return true;
	}
	return false;
}

Scalar DrawMolecule::Intersects(const DrawMolecule& other) const {
	Scalar intersectionAmount = 0;
	auto transform = GetState();
	auto otherTransform = other.GetState();
	for (const auto& atom : molecule->GetAtoms()) {
		auto position = transform.transform(atom->position);

		for(const auto& otherAtom : other.molecule->GetAtoms()) {
			auto otherPosition = otherTransform.transform(otherAtom->position);
			auto dist2 = Geometry::sqrDist(position, otherPosition);
			auto mindist2 = Math::sqr(atom->radius + otherAtom->radius);
			if (dist2 <= mindist2 && mindist2 - dist2 > intersectionAmount)
				intersectionAmount = mindist2 - dist2;
		}
	}
	return intersectionAmount;
}

bool DrawMolecule::Lock() {
	if (!locked)
		return locked = true;
	return false;
}

void DrawMolecule::Unlock() {
	locked = false;
}

ONTransform DrawMolecule::GetState() const {
	return ONTransform(position - Point::origin, orientation);
}

void DrawMolecule::SetState(const ONTransform& newState) {
	position = newState.getOrigin();
	orientation = newState.getRotation();
	velocity = Vector::zero;
	angularVelocity = Vector::zero;
}

const Point& DrawMolecule::GetPosition() const {
	return position;
}

void DrawMolecule::Step(const Vector& netForce, const Vector& netTorque, Scalar timeStep) {
	const Scalar mass = 1;
	const Scalar inertia = 1;
	Scalar t2 = timeStep * timeStep;

	Vector linearAccel = netForce / mass;
	Vector angularAccel = netTorque * (mass / inertia);

	// Clamp accelerations
	Scalar max = linearAccel.max();
	if (max > 100) {
		linearAccel *= 100 / max;
	}
	max = angularAccel.max();
	if (max > 100) {
		angularAccel *= 100 / max;
	}

	// Update linear
	position += velocity * timeStep + linearAccel * t2 * 0.5;
	velocity += linearAccel * timeStep;

	// Update angular
	orientation.leftMultiply(Rotation(angularVelocity * timeStep + angularAccel * t2 * 0.5));
	angularVelocity += angularAccel * t2;

	// Clamp position inside domain box
	for(int i = 0; i < 3; i++) {
		position[i] = Math::clamp(position[i], Scalar(-40), Scalar(40));
	}
}

void DrawMolecule::ResetForces() {
	//std::cout << "Reset forces" << std::endl;
	velocity = Vector::zero;
	angularVelocity = Vector::zero;
}

void DrawMolecule::initContext(GLContextData& contextData) const {
	/* Create a context data item and store it in the GLContextData object: */
	DataItem* dataItem = new DataItem;
	contextData.addDataItem(this, dataItem);

	if (dataItem->hasVertexBufferObjectExtension) {
		/* Upload the (mostly invariant) index buffer data for all crystal faces: */
		/*
		for (int faceIndex = 0; faceIndex < 6; ++faceIndex) {
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
					dataItem->faceIndexBufferObjectIDs[faceIndex]);

			//glBufferDataARB(GLenum target, GLsizeiptrARB size, const GLvoid* data, GLenum usage)
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
					numVertices[faceIndex][0] * 2 * (numVertices[faceIndex][1] - 1) * sizeof(GLuint),
							indices[faceIndex],
							GL_STATIC_DRAW_ARB);
		}
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		*/

		/*
		auto that = const_cast<DrawMolecule*>(this);
		that->ComputeSurf();

		glBegin(GL_TRIANGLES);
		for (auto& v : vertices) {
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, v->color);
			glVertex(*v);
		}
		glEnd();
		*/
	}
}

void DrawMolecule::glRenderAction(GLContextData& contextData) const {
	glPushMatrix();
	glMultMatrix(GetState());

	switch (style) {
	case DrawStyle::Points:
		DrawPoints(contextData);
		break;
	case DrawStyle::Surf:
		DrawSurf(contextData);
		break;
	case DrawStyle::None:
		throw std::runtime_error("DrawStyle::None is not supported");
	}

	glPopMatrix();
}

/**
 * Dibujar molecula usando esferas.
 */
void DrawMolecule::DrawPoints(GLContextData& contextData) const {
	/* Get the OpenGL-dependent application data from the GLContextData object: */
	DataItem* dataItem = contextData.retrieveDataItem<DataItem>(this);

	if (dataItem->displayListDrawStyle == DrawStyle::Points
			&& dataItem->displayListColorStyle == colorStyle) {
		/* Call the display list */
		glCallList(dataItem->displayListId);
	}
	else {
		glDeleteLists(dataItem->displayListId, 1);
		dataItem->displayListDrawStyle = DrawStyle::Points;
		dataItem->displayListColorStyle = colorStyle;

		/* Compile and execute a new display list */
		std::cout << "Compiling new display list" << std::endl;
		glNewList(dataItem->displayListId, GL_COMPILE_AND_EXECUTE);
		{
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, Color(0.9f, 0.9f, 0.9f));
			for (const auto& a : molecule->GetAtoms()) {
				if (colorStyle == ColorStyle::AnaglyphFriendly || colorStyle == ColorStyle::CPK) {
					auto color = AtomColor(a->short_name);
					glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, color);
				}
				else if (colorStyle == ColorStyle::Pockets) {
					auto color = AtomColor(a->serial);
					glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, color);
				}

				glPushMatrix();
				glTranslate(a->position - Point::origin);
				glDrawSphereIcosahedron(a->radius, 1);
				glPopMatrix();
			}
		}
		glEndList();
	}
}

/**
 * Dibujar usando superficie.
 */
void DrawMolecule::DrawSurf(GLContextData& contextData) const {
	if (!surfComputed)
		throw std::runtime_error("need to compute surf before draw");

	/* Get the OpenGL-dependent application data from the GLContextData object: */
	DataItem* dataItem = contextData.retrieveDataItem<DataItem>(this);

	if (dataItem->displayListDrawStyle == DrawStyle::Surf
			&& dataItem->displayListColorStyle == colorStyle) {
		/* Call the display list */
		glCallList(dataItem->displayListId);
	}
	else {
		glDeleteLists(dataItem->displayListId, 1);
		dataItem->displayListDrawStyle = DrawStyle::Surf;
		dataItem->displayListColorStyle = colorStyle;

		/* Compile and execute a new display list */
		std::cout << "Compiling new display list" << std::endl;
		glNewList(dataItem->displayListId, GL_COMPILE_AND_EXECUTE);
		{
			glBegin(GL_TRIANGLES);
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, Color(0.9f, 0.9f, 0.9f));
			for (auto& v : vertices) {
				if (colorStyle == ColorStyle::AnaglyphFriendly || colorStyle == ColorStyle::CPK) {
					// The atom info is encoded inside the color components (i know...)
					char name = (char)(v->color[0] * 256.0f);
					auto atomColor = AtomColor(name);
					glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, atomColor);
				}
				else if (colorStyle == ColorStyle::Pockets) {
					int serial = (int)(v->color[1] * 16384.0f);
					auto atomColor = AtomColor(serial);
					glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, atomColor);
				}
				glVertex(*v);
			}
			glEnd();
		}
		glEndList();
	}
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
			for (int i = 0; i < 3; i++) {
				if (!getline(infile, line))
					throw std::runtime_error("Error reading .tri");
				float x, y, z, nx, ny, nz;
				if (6 != sscanf(line.c_str(), "%f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz))
					throw std::runtime_error("Error parsing .tri");

				auto vertex = unique_ptr<Vertex>(new Vertex());
				//vertex->color = *AtomColor(molecule->GetAtoms()[atom_id]->short_name).release();
				// The atom info is encoded inside the color components (i know...)
				auto& atom = molecule->GetAtoms()[atom_id];
				char name = atom->short_name;
				int serial = atom->serial;
				vertex->color = Vertex::Color(name/256.0f, serial/16384.0f, 0);
				vertex->normal = Vertex::Normal(nx, ny, nz);
				vertex->position = Vertex::Position(x, y, z);

				vertices.push_back(move(vertex));
			}
		}
		infile.close();

		cout << "Surf computed. Loaded " << (vertices.size() / 3) << " triangles." << endl;
	}
	else {
		cout << "not found." << endl;
		throw std::runtime_error("TODO: Implement surf");
	}
	surfComputed = true;
}

void DrawMolecule::ComputePockets() {
	if (pocketsComputed)
		return;
	// Ver si existe un archivo .pocket ya generado por pocket
	cout << "Trying to open " << molecule->source_filename << ".pocket ...";
	ifstream infile;
	infile.open(molecule->source_filename + ".pocket");
	if (!infile.fail()) {
		cout << "success." << endl;

		string line;
		while (getline(infile, line)) {
			if (line.substr(0, 5) == "ATOM ") {
				int atomSerial = stoi(line.substr(6, 5));
				int pocketId = stoi(line.substr(70, 2));
				pockets[atomSerial] = pocketId;
			}
		}
		infile.close();

		cout << "Pockets computed. Loaded " << pockets.size() << " pocket atoms." << endl;
	}
	else {
		cout << "not found." << endl;
		//throw std::runtime_error("TODO: Implement pocket");
		cout << "Warning: No pockets loaded for molecule." << std::endl;
	}
	pocketsComputed = true;
}

DrawMolecule::Color DrawMolecule::AtomColor(char short_name) const {
	if (colorStyle == ColorStyle::AnaglyphFriendly) {
		// Anaglyph-friendly coloring
		switch (short_name) {
		case 'H':
			return Color(0.8f, 0.0f, 0.4f); // red
		case 'C':
			return Color(0.3f, 0.8f, 0.3f); // green
		case 'N':
			return Color(0.8f, 0.8f, 0.6f); // yellow
		case 'O':
			return Color(0.5f, 0.3f, 0.8f); // blue
		}
		return Color(0.9f, 0.9f, 0.9f); // white
	}
	if (colorStyle == ColorStyle::CPK) {
		// CPK coloring (http://en.wikipedia.org/wiki/CPK_coloring)
		switch (short_name) {
		case 'H':
			return Color(0.95f, 0.95f, 0.95f); // white
		case 'C':
			return Color(0.65f, 0.65f, 0.65f); // gray
		case 'N':
			return Color(34/255.0f, 51/255.0f, 1); // dark blue
		case 'O':
			return Color(0.94f, 0, 0); // red
		case 'S':
			return Color(1, 200/255.0f, 50/255.0f); // orange
		return Color(221/255.0f, 119/255.0f, 1); // pink
		}
	}
	// default for None
	return Color(0.9f, 0.9f, 0.9f); // white
}

DrawMolecule::Color DrawMolecule::AtomColor(int serial) const {
	if (colorStyle != ColorStyle::Pockets)
		throw std::runtime_error("Bad call to DrawMolecule::AtomColor");

	auto it = pockets.find(serial);
	if (it != pockets.end()) {
		switch(it->second) {
		case 1:
			return Color(0.0f, 0.0f, 1.0f);
		case 2:
			return Color(0.0f, 1.0f, 0.0f);
		case 3:
			return Color(0.0f, 1.0f, 1.0f);
		case 4:
			return Color(1.0f, 0.0f, 0.0f);
		case 5:
			return Color(1.0f, 0.0f, 1.0f);
		}
		//return Color(221/255.0f, 119/255.0f, 1); // pink
		return Color(0.7f, 0.7f, 0.7f);
	}
	// default
	return Color(0.9f, 0.9f, 0.9f); // white
}

const Molecule& DrawMolecule::GetMolecule() const {
	return *molecule;
}

const Point& DrawMolecule::GetCenter() {
	return molecule->GetCenter();
}

std::string DrawMolecule::GetName() const {
	return molecule->source_filename;
}

ColorStyle DrawMolecule::GetColorStyle() const {
	return colorStyle;
}

void DrawMolecule::SetColorStyle(ColorStyle colorStyle) {
	this->colorStyle = colorStyle;
	if (colorStyle == ColorStyle::Pockets) {
		ComputePockets();
	}
}

DrawStyle DrawMolecule::GetDrawStyle() const {
	return style;
}

void DrawMolecule::SetDrawStyle(DrawStyle style) {
	this->style = style;
	if (style == DrawStyle::Surf) {
		ComputeSurf();
	}
}

}
