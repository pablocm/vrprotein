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
#include <GL/GLColor.h>
#include <GL/GLObject.h>
#include <GL/GLVertex.h>
#include <Vrui/TransparentObject.h>
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

class DrawMolecule: public GLObject, public Vrui::TransparentObject {
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
		unsigned int glDataVersion; // Version number of the graphics data stored in this GLContext.

		/* Constructors and destructors: */
		DataItem(void);

		virtual ~DataItem(void);
	};

	/* Public Methods: */
	DrawMolecule(std::unique_ptr<Molecule> m);
	DrawMolecule(const DrawMolecule&) = delete;				// delete copy ctor
	DrawMolecule& operator=(DrawMolecule const&) = delete; 	// delete assignment
	bool intersects(const Ray& r) const;
	bool intersects(const Point& p) const;
	Scalar intersects(const DrawMolecule& other) const;
	bool lock();
	void unlock();
	ONTransform getState() const;	// Returns position and orientation of molecule
	void setState(const ONTransform& newState); // Sets state of molecule. Atom must be locked by caller.
	void step(const Vector& netForce, const Vector& netTorque, Scalar timeStep);
	void resetForces();

	virtual void initContext(GLContextData& contextData) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	virtual void glRenderActionTransparent(GLContextData& contextData) const;
	void computeSurf();
	void computePockets();
	const Molecule& getMolecule() const;
	std::string getName() const;
	DrawStyle getDrawStyle() const;
	void setDrawStyle(DrawStyle style);
	ColorStyle getColorStyle() const;
	void setColorStyle(ColorStyle useColor);
	const std::unordered_map<int, Point>& getPocketCentroids() const;
	const std::vector<Sphere>& getSpheresOfPocket(int pocket) const;
	std::string getNameOfPocket(int pocket) const;
	bool getTransparency() const;
	void setTransparency(bool isTransparent);
	bool getVisibility() const;
	void setVisibility(bool isVisible);
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
	bool isVisible;
	bool isTransparent;
	GLfloat alpha;
	bool locked;	// currently locked by a dragger
	unsigned int glDataVersion; // Version number of the graphics data stored in the GLContexts.

	bool computeSurfIdx();
	void draw(GLContextData& contextData) const;
	void drawPoints(GLContextData& contextData) const;
	void drawSurf(GLContextData& contextData) const;
	DrawMolecule::Color atomColor(char short_name) const;
	DrawMolecule::Color atomColor(int serial) const;
	void updateVerticesColors();	// Update vertices colors (when using indices)
};

}

#endif /* DRAWMOLECULE_H_ */
