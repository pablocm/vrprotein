/*
 * DrawMolecule.h
 *
 *  Created on: Mar 16, 2014
 *      Author: pablocm
 */

#ifndef DRAWMOLECULE_H_
#define DRAWMOLECULE_H_

#include <memory>
#include <unordered_map>
#include <GL/GLObject.h>
#include <GL/GLColor.h>
#include <GL/GLVertex.h>
#include "AffineSpace.h"
#include "Molecule.h"

/* Forward declarations: */
class GLContextData;

namespace VrProtein {

enum class DrawStyle {
	None = 0, Points, Surf
};

enum class ColorStyle {
	None = 0, AnaglyphFriendly, CPK, Pockets
};

class DrawMolecule: public GLObject {
public:
	/* Embedded classes: */
	typedef GLVertex<void, 0, GLfloat, 4, GLfloat, GLfloat, 3> Vertex;
	typedef GLColor<GLfloat, 4> Color;

	struct DataItem: public GLObject::DataItem {
	public:
		// DrawStyle: Surf
		bool hasVertexBufferObjectExtension; // Flag if the local OpenGL supports the ARB vertex buffer object extension
		GLuint faceVertexBufferObjectIDs[1]; // Array of vertex buffer object IDs
		GLuint faceIndexBufferObjectIDs[1]; // Array of index buffer object IDs

		GLuint displayListId;  // The display List for the molecule
		DrawStyle displayListDrawStyle;  // Selected style for current display list
		ColorStyle displayListColorStyle;  // Selected color style for current display list

		/* Constructors and destructors: */
		DataItem(void);

		virtual ~DataItem(void);
	};

	/* Public Methods: */
	DrawMolecule(std::unique_ptr<Molecule> m);
	DrawMolecule(const DrawMolecule&) = delete;				// delete copy ctor
	DrawMolecule& operator=(DrawMolecule const&) = delete; 	// delete assignment
	bool Intersects(const Ray& r) const;
	bool Intersects(const Point& p) const;
	Scalar Intersects(const DrawMolecule& other) const;
	bool Lock();
	void Unlock();
	ONTransform GetState() const;	// Returns position and orientation of molecule
	void SetState(const ONTransform& newState); // Sets state of molecule. Atom must be locked by caller.
	void Step(const Vector& netForce, const Vector& netTorque, Scalar timeStep);
	void ResetForces();

	virtual void initContext(GLContextData& contextData) const;
	void glRenderAction(GLContextData& contextData) const;
	void ComputeSurf();
	void ComputePockets();
	const Molecule& GetMolecule() const;
	std::string GetName() const;
	DrawStyle GetDrawStyle() const;
	void SetDrawStyle(DrawStyle style);
	ColorStyle GetColorStyle() const;
	void SetColorStyle(ColorStyle useColor);
	const std::unordered_map<int, Point>& GetPocketCentroids() const;
	const std::vector<Sphere>& GetSpheresOfPocket(int pocket) const;
	std::string GetNameOfPocket(int pocket) const;
private:
	std::unique_ptr<Molecule> molecule;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::unordered_map<int, int> vertexToAtom;					// <vertex id, Atom Serial>
	std::unordered_map<int, int> atomToPocket;					// <Atom Serial, pocket ID>
	std::unordered_map<int, std::vector<int>> pocketToAtoms;	// <pocket ID, atom serials list>
	std::unordered_map<int, std::vector<Sphere>> pocketToSpheres;//<pocket ID, Sphere>
	std::unordered_map<int, Point> pocketCentroids;				// <pocket ID, centroid>
	Point position;
	Rotation orientation;
	Vector velocity;
	Vector angularVelocity;
	bool surfComputed;
	bool surfUsesIndices;
	bool pocketsComputed;
	DrawStyle style;
	ColorStyle colorStyle;
	bool locked;	// currently locked by a dragger

	bool ComputeSurfIdx();
	void DrawPoints(GLContextData& contextData) const;
	void DrawSurf(GLContextData& contextData) const;
	DrawMolecule::Color AtomColor(char short_name) const;
	DrawMolecule::Color AtomColor(int serial) const;
};

}

#endif /* DRAWMOLECULE_H_ */
