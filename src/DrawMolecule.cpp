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
#include <Vrui/Vrui.h>
#include "DrawMolecule.h"
#include "Molecule.h"

using namespace std;

namespace VrProtein {

/****************************************
Methods of class DrawMolecule::DataItem:
****************************************/

DrawMolecule::DataItem::DataItem() :
			hasVertexBufferObjectExtension(GLARBVertexBufferObject::isSupported()),
			glDataVersion(0) {
	if (hasVertexBufferObjectExtension) {
		//std::cout << "Video card supports GL_ARB_vertex_buffer_object." << std::endl;
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();

		/* Create the vertex and index buffer objects: */
		glGenBuffersARB(1, faceVertexBufferObjectIDs);
		glGenBuffersARB(1, faceIndexBufferObjectIDs);
	}
	else {
		//std::cout << "Video card does NOT support GL_ARB_vertex_buffer_object." << std::endl;
	}

	// Create display list for storing molecule rendering
	displayListId = glGenLists(1);
}

DrawMolecule::DataItem::~DataItem(void) {
	if (hasVertexBufferObjectExtension) {
		/* Destroy the vertex and index buffer objects: */
		glDeleteBuffersARB(1, faceVertexBufferObjectIDs);
		glDeleteBuffersARB(1, faceIndexBufferObjectIDs);
	}

	// Destroy the display list
	glDeleteLists(displayListId, 1);
}

/*****************************
Methods of class DrawMolecule:
*****************************/

DrawMolecule::DrawMolecule(unique_ptr<Molecule> m) {
	molecule = move(m);
	molecule->fixCenterOffset();
	style = DrawStyle::Points;
	surfComputed = false;
	surfUsesIndices = false;
	pocketsComputed = false;
	colorStyle = ColorStyle::CPK;
	isVisible = true;
	isTransparent = false;
	alpha = 1.0f;
	locked = false;
	velocity = Vector::zero;
	position = Point::origin;
	orientation = Rotation::identity;
	glDataVersion = 0;
}

bool DrawMolecule::intersects(const Ray& r) const {
	auto transform = getState();
	Sphere sphere(Point::origin, 0);  // Test sphere
	for (const auto& atom : molecule->getAtoms()) {
		auto pos = transform.transform(atom->position);
		sphere.setCenter(pos);
		sphere.setRadius(atom->radius * 1.5);

		auto hitResult = sphere.intersectRay(r);

		if (hitResult.isValid())
			return true;
	}
	return false;
}

bool DrawMolecule::intersects(const Point& p) const {
	auto transform = getState();
	for (const auto& atom : molecule->getAtoms()) {
		auto pos = transform.transform(atom->position);

		auto dist2 = Geometry::sqrDist(p, pos);
		if (dist2 <= Math::sqr(atom->radius * 1.5))
			return true;
	}
	return false;
}

Scalar DrawMolecule::intersects(const DrawMolecule& other) const {
	//Scalar intersectionAmount2 = 0;
	Scalar intersectionAmount = 0;
	auto transform = getState();
	auto otherTransform = other.getState();
	for (const auto& atom : molecule->getAtoms()) {
		auto pos = transform.transform(atom->position);

		for(const auto& otherAtom : other.molecule->getAtoms()) {
			auto otherPosition = otherTransform.transform(otherAtom->position);
			auto dist2 = Geometry::sqrDist(pos, otherPosition);
			auto mindist2 = Math::sqr(atom->radius + otherAtom->radius);
			//if (mindist2 - dist2 > intersectionAmount2) { // TODO: Check math
			//	intersectionAmount2 = mindist2 - dist2;
			//	intersectionAmount = Math::sqrt(mindist2) - Math::sqrt(dist2);
			//}
			if (dist2 < mindist2)
				intersectionAmount += Math::sqrt(mindist2) - Math::sqrt(dist2);
		}
	}
	return intersectionAmount;
}

bool DrawMolecule::lock() {
	if (!locked)
		return locked = true;
	return false;
}

void DrawMolecule::unlock() {
	locked = false;
}

ONTransform DrawMolecule::getState() const {
	return ONTransform(position - Point::origin, orientation);
}

void DrawMolecule::setState(const ONTransform& newState) {
	position = newState.getOrigin();
	orientation = newState.getRotation();
	velocity = Vector::zero;
	angularVelocity = Vector::zero;
}

void DrawMolecule::step(const Vector& netForce, const Vector& netTorque, Scalar timeStep) {
	const Scalar mass = 1;
	const Scalar inertia = 2;
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

	// Attenuate velocities over time
	velocity *= 0.95;
	angularVelocity *= 0.95;
}

void DrawMolecule::resetForces() {
	//std::cout << "Reset forces" << std::endl;
	velocity = Vector::zero;
	angularVelocity = Vector::zero;
}

void DrawMolecule::initContext(GLContextData& contextData) const {
	/* Create a context data item and store it in the GLContextData object: */
	DataItem* dataItem = new DataItem;
	contextData.addDataItem(this, dataItem);
}

void DrawMolecule::draw(GLContextData& contextData) const {
	glPushMatrix();
	/* Go to model coordinates: */
	glMultMatrix(getState());

	switch (style) {
	case DrawStyle::Points:
		drawPoints(contextData);
		break;
	case DrawStyle::Surf:
		drawSurf(contextData);
		break;
	case DrawStyle::None:
		throw std::runtime_error("DrawStyle::None is not supported");
	default:
		throw std::runtime_error("Unknown DrawStyle");
	}

	/* Draw Pocket centroids:
	glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, Color(0.9f, 0.3f, 0.3f, 1.0f));
	for (const auto& it : pocketCentroids) {
		glPushMatrix();
		glTranslate(it.second - Point::origin);
		glDrawSphereIcosahedron(1, 1);
		glPopMatrix();
	}
	*/
	glPopMatrix();
}


void DrawMolecule::glRenderAction(GLContextData& contextData) const {
	if (isVisible && !isTransparent)
		draw(contextData);
}

void DrawMolecule::glRenderActionTransparent(GLContextData& contextData) const {
	if (isVisible && isTransparent) {
		glPushMatrix();
		/* Go to navigation coordinates: */
		glMultMatrix(Vrui::getNavigationTransformation());

		/* Save and prepare OpenGL state to render transparency: */
		glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

		/* Render all back faces first: */
		glCullFace(GL_FRONT);
		draw(contextData);
		/* Render the front faces next: */
		glCullFace(GL_BACK);
		draw(contextData);

		glPopAttrib();
		glPopMatrix();
	}
}

/**
 * Dibujar molecula usando esferas.
 */
void DrawMolecule::drawPoints(GLContextData& contextData) const {
	/* Get the OpenGL-dependent application data from the GLContextData object: */
	DataItem* dataItem = contextData.retrieveDataItem<DataItem>(this);

	if (dataItem->glDataVersion == glDataVersion) {
		/* Call the display list */
		glCallList(dataItem->displayListId);
	}
	else {
		glDeleteLists(dataItem->displayListId, 1);
		dataItem->glDataVersion = glDataVersion;

		/* Compile and execute a new display list */
		//std::cout << "Compiling new display list" << std::endl;
		glNewList(dataItem->displayListId, GL_COMPILE_AND_EXECUTE);
		{
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT_AND_BACK, Color(0.9f, 0.9f, 0.9f, alpha));
			for (const auto& a : molecule->getAtoms()) {
				if (colorStyle == ColorStyle::AnaglyphFriendly || colorStyle == ColorStyle::CPK) {
					auto color = atomColor(a->short_name);
					glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT_AND_BACK, color);
				}
				else if (colorStyle == ColorStyle::Pockets) {
					auto color = atomColor(a->serial);
					glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT_AND_BACK, color);
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
void DrawMolecule::drawSurf(GLContextData& contextData) const {
	if (!surfComputed)
		throw std::runtime_error("need to compute surf before draw");

	/* Get the OpenGL-dependent application data from the GLContextData object: */
	DataItem* dataItem = contextData.retrieveDataItem<DataItem>(this);

	if (surfUsesIndices) {
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_COLOR_MATERIAL);
		GLVertexArrayParts::enable(Vertex::getPartsMask());
		const GLuint* indexPtr;

		if (dataItem->hasVertexBufferObjectExtension) {
			indexPtr = 0;
			/* Bind the face's index buffer object: */
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, dataItem->faceIndexBufferObjectIDs[0]);
			/* Bind the face's vertex buffer object: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, dataItem->faceVertexBufferObjectIDs[0]);
			/* Check if we need to upload data: */
			if (dataItem->glDataVersion != glDataVersion) {
				//std::cout << "Updating VBO" << std::endl;
				dataItem->glDataVersion = glDataVersion;
				/* Upload new vertex & index data: */
				glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indices.size() * sizeof(GLuint),
						&indices[0], GL_STATIC_DRAW_ARB);
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertices.size() * sizeof(Vertex), &vertices[0],
						GL_STATIC_DRAW_ARB);
			}
			glVertexPointer(static_cast<const Vertex*>(0));
		}
		else {
			/* Fall back to using regular vertex arrays (ouch): */
			indexPtr = &indices[0];
			glVertexPointer(&vertices[0]);
		}

		/* Render the surface a sequence of triangles: */
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indexPtr);

		if (dataItem->hasVertexBufferObjectExtension) {
			/* Unbind all buffers: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		}
		GLVertexArrayParts::disable(Vertex::getPartsMask());
		glPopAttrib();
	}
	else {
		if (dataItem->glDataVersion == glDataVersion) {
			/* Call the display list */
			glCallList(dataItem->displayListId);
		}
		else {
			glDeleteLists(dataItem->displayListId, 1);
			dataItem->glDataVersion = glDataVersion;

			/* Compile and execute a new display list */
			//std::cout << "Compiling new display list" << std::endl;
			glNewList(dataItem->displayListId, GL_COMPILE_AND_EXECUTE);
			{
				glBegin(GL_TRIANGLES);
				glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT_AND_BACK, Color(0.9f, 0.9f, 0.9f, alpha));
				for (const auto& v : vertices) {
					if (colorStyle == ColorStyle::AnaglyphFriendly || colorStyle == ColorStyle::CPK) {
						// The atom info is encoded inside the color components (i know...)
						char name = static_cast<char>(v.color[0] * 256.0f);
						auto color = atomColor(name);
						glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT_AND_BACK, color);
					}
					else if (colorStyle == ColorStyle::Pockets) {
						int serial = static_cast<int>(v.color[1] * 16384.0f);
						auto color = atomColor(serial);
						glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT_AND_BACK, color);
					}
					glVertex(v);
				}
				glEnd();
			}
			glEndList();
		}
	}
}

void DrawMolecule::computeSurf() {
	if (surfComputed)
		return;
	if (computeSurfIdx())
		return;

	// Ver si existe un archivo .tri ya generado por SURF
	cout << "Trying to open " << molecule->source_filename << ".tri ...";
	ifstream infile;
	infile.open(molecule->source_filename + ".tri");
	if (!infile.fail()) {
		cout << "success." << endl;

		const Vector offset = molecule->getOffset();
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


				Vertex vertex;
				//vertex->color = *AtomColor(molecule->GetAtoms()[atom_id]->short_name).release();
				// The atom info is encoded inside the color components (i know...)
				auto& atom = molecule->getAtoms().at(atom_id);
				char name = atom->short_name;
				int serial = atom->serial;
				vertex.color = Vertex::Color(name/256.0f, serial/16384.0f, 0, alpha);
				vertex.normal = Vertex::Normal(nx, ny, nz);
				vertex.position = Vertex::Position(x + offset[0], y + offset[1], z + offset[2]);

				vertices.push_back(vertex);
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
	surfUsesIndices = false;
	glDataVersion++;
}

bool DrawMolecule::computeSurfIdx() {
	cout << "Trying to open " << molecule->source_filename << ".tri.idx ...";
	ifstream infile;
	infile.open(molecule->source_filename + ".tri.idx");
	if (!infile.fail()) {
		cout << "success." << endl;

		const Vector offset = molecule->getOffset();
		string line;

		// Check file header
		getline(infile, line);
		if (line != "VERTICES")
			throw std::runtime_error("Error parsing file");

		// Read vertices
		while (getline(infile, line) && line != "TRIANGLES") {
			int id;
			float x, y, z, nx, ny, nz;
			if (7 != sscanf(line.c_str(), "%d %f %f %f %f %f %f", &id, &x, &y, &z, &nx, &ny, &nz))
				throw std::runtime_error("Error reading .tri.idx vertices");

			Vertex vertex;
			vertex.color = Vertex::Color(0.0f, 0.0f, 0.0f, alpha); // Unknown at this point
			vertex.normal = Vertex::Normal(nx, ny, nz);
			vertex.position = Vertex::Position(x + offset[0], y + offset[1], z + offset[2]);
			vertices.push_back(vertex);
		}

		if (line != "TRIANGLES")
			throw std::runtime_error("Error parsing file");

		// Read indexed triangles
		const auto& atoms = molecule->getAtoms();
		while (getline(infile, line)) {
			int atom_id = stoi(line);
			// push 3 indices
			for (int i = 0; i < 3; i++) {
				if (!getline(infile, line))
					throw std::runtime_error("Error reading .tri.idx triangles");
				int vIndex = stoi(line);
				indices.push_back(vIndex);
				// TODO: Will overwrite id for shared vertices
				vertexToAtom[vIndex] = atoms.at(atom_id)->serial;
			}
		}
		infile.close();

		cout << "Surf computed. Vertices: " << vertices.size();
		cout << "; Indices: " << indices.size() << endl;
		surfComputed = true;
		surfUsesIndices = true;
		glDataVersion++;

		// Colorize vertices
		updateVerticesColors();
	}
	else {
		cout << "not found." << endl;
	}
	return surfComputed;
}

void DrawMolecule::computePockets() {
	const int maxPockets = 5;	// max pockets per molecule
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
				if (pocketId > maxPockets)
					break;
				// add to dictionaries
				atomToPocket[atomSerial] = pocketId;
				pocketToAtoms[pocketId].push_back(atomSerial);
				auto& atom = molecule->findBySerial(atomSerial);
				pocketToSpheres[pocketId].push_back(Sphere(atom->position, atom->radius));
			}
		}
		infile.close();

		// Calculate centroids of pockets
		for(const auto& it : pocketToSpheres) {
			auto centroid = Vector::zero;
			for(auto sphere : it.second) {
				centroid += (sphere.getCenter() - Point::origin);
			}
			centroid /= it.second.size();
			pocketCentroids[it.first] = Point(centroid);
		}

		cout << "Pockets computed. Loaded " << pocketToAtoms.size() << " pockets." << endl;
	}
	else {
		cout << "not found." << endl;
		cout << "Warning: No pockets loaded for molecule." << std::endl;
	}
	pocketsComputed = true;
}

DrawMolecule::Color DrawMolecule::atomColor(char short_name) const {
	if (colorStyle == ColorStyle::AnaglyphFriendly) {
		// Anaglyph-friendly coloring
		switch (short_name) {
		case 'H':
			return Color(0.8f, 0.0f, 0.4f, alpha); // red
		case 'C':
			return Color(0.3f, 0.8f, 0.3f, alpha); // green
		case 'N':
			return Color(0.8f, 0.8f, 0.6f, alpha); // yellow
		case 'O':
			return Color(0.5f, 0.3f, 0.8f, alpha); // blue
		default:
			return Color(0.9f, 0.9f, 0.9f, alpha); // white
		}
	}
	if (colorStyle == ColorStyle::CPK) {
		// CPK coloring (http://en.wikipedia.org/wiki/CPK_coloring)
		switch (short_name) {
		case 'H':
			return Color(0.95f, 0.95f, 0.95f, alpha); // white
		case 'C':
			return Color(0.65f, 0.65f, 0.65f, alpha); // gray
		case 'N':
			return Color(34/255.0f, 51/255.0f, 1, alpha); // dark blue
		case 'O':
			return Color(0.94f, 0, 0, alpha); // red
		case 'S':
			return Color(1, 200/255.0f, 50/255.0f, alpha); // orange
		default:
			return Color(221/255.0f, 119/255.0f, 1, alpha); // pink
		}
	}
	// default for None
	return Color(0.9f, 0.9f, 0.9f, alpha); // white
}

DrawMolecule::Color DrawMolecule::atomColor(int serial) const {
	if (colorStyle != ColorStyle::Pockets)
		//throw std::runtime_error("Bad call to DrawMolecule::AtomColor");
		return atomColor(molecule->findBySerial(serial)->short_name);

	auto it = atomToPocket.find(serial);
	if (it != atomToPocket.end()) {
		// randomize color
		int randomId = (it->second + molecule->getAtoms().size()) % 5 + 1;
		switch(randomId) {
		case 1:
			return Color(0.3f, 0.3f, 1.0f, alpha);
		case 2:
			return Color(0.3f, 1.0f, 0.3f, alpha);
		case 3:
			return Color(0.3f, 1.0f, 1.0f, alpha);
		case 4:
			return Color(1.0f, 0.3f, 0.3f, alpha);
		case 5:
			return Color(1.0f, 0.3f, 1.0f, alpha);
		default:
			return Color(0.7f, 0.7f, 0.7f, alpha);
		}
		//return Color(221/255.0f, 119/255.0f, 1, alpha); // pink
	}
	// default
	return Color(0.9f, 0.9f, 0.9f, alpha); // white
}

std::string DrawMolecule::getNameOfPocket(int pocket) const {
	if (pocket == -1)
		return "---";

	int randomId = (pocket + molecule->getAtoms().size()) % 5 + 1;
	switch (randomId) {
	case 1:
		return "blue";
	case 2:
		return "green";
	case 3:
		return "cyan";
	case 4:
		return "red";
	case 5:
		return "pink";
	default:
		return "??";
	}
}

const Molecule& DrawMolecule::getMolecule() const {
	return *molecule;
}

std::string DrawMolecule::getName() const {
	return molecule->source_filename;
}

ColorStyle DrawMolecule::getColorStyle() const {
	return colorStyle;
}

void DrawMolecule::setColorStyle(ColorStyle newColorStyle) {
	if (colorStyle == newColorStyle)
		return;

	colorStyle = newColorStyle;
	if (colorStyle == ColorStyle::Pockets) {
		computePockets();
	}
	updateVerticesColors();
	glDataVersion++;
}

DrawStyle DrawMolecule::getDrawStyle() const {
	return style;
}

void DrawMolecule::setDrawStyle(DrawStyle newStyle) {
	if (style == newStyle)
		return;

	style = newStyle;
	if (style == DrawStyle::Surf) {
		computeSurf();
	}
	glDataVersion++;
}

void DrawMolecule::updateVerticesColors() {
	if (surfComputed && surfUsesIndices) {
		switch(colorStyle) {
		case ColorStyle::AnaglyphFriendly:
		case ColorStyle::CPK:
		case ColorStyle::Pockets:
			for (unsigned int i = 0; i < vertices.size(); i++) {
				vertices[i].color = atomColor(vertexToAtom.at(i));
				vertices[i].color[3] = isTransparent ? 0.6f : 1.0f;
			}
			break;
		case ColorStyle::None:
			for (auto& v : vertices) {
				v.color[0] = 0.9f;
				v.color[1] = 0.9f;
				v.color[2] = 0.9f;
				v.color[3] = isTransparent ? 0.6f : 1.0f;
			}
			break;
		default:
			throw std::runtime_error("Unknown colorStyle");
		}
		glDataVersion++;
	}
}

bool DrawMolecule::getTransparency() const {
	return isTransparent;
}

void DrawMolecule::setTransparency(bool newIsTransparent) {
	if (isTransparent == newIsTransparent)
		return;

	isTransparent = newIsTransparent;
	if (isTransparent)
		alpha = 0.4f;
	else
		alpha = 1.0f;
	updateVerticesColors();
	glDataVersion++;
}

bool DrawMolecule::getVisibility() const {
	return isVisible;
}

void DrawMolecule::setVisibility(bool newIsVisible) {
	isVisible = newIsVisible;
}

const std::unordered_map<int, Point>& DrawMolecule::getPocketCentroids() const {
	return pocketCentroids;
}

const std::vector<Sphere>& DrawMolecule::getSpheresOfPocket(int pocket) const {
	return pocketToSpheres.at(pocket);
}

}
