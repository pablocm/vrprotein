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
		GLuint faceVertexBufferObjectIDs[6]; // Array of vertex buffer object IDs for the Jell-O faces
		GLuint faceIndexBufferObjectIDs[6]; // Array of index buffer object IDs for the Jell-O faces
		unsigned int vertexDataVersion; // Version number of the face data in the vertex buffers

		GLuint displayListId;  // The display List for the molecule
		DrawStyle displayListDrawStyle;  // Selected style for current display list
		ColorStyle displayListColorStyle;  // Selected color style for current display list

		/* Constructors and destructors: */
		DataItem(void);

		virtual ~DataItem(void);
	};

	/* Public Methods: */
	DrawMolecule(std::unique_ptr<Molecule> m);
	bool Intersects(const Ray& r) const;
	bool Intersects(const Point& p) const;
	Scalar Intersects(const DrawMolecule& other) const;
	bool Lock();
	void Unlock();
	ONTransform GetState() const;	// Returns position and orientation of molecule
	void SetState(const ONTransform& newState); // Sets state of molecule. Atom must be locked by caller.
	const Point& GetPosition() const;

	virtual void initContext(GLContextData& contextData) const;
	void glRenderAction(GLContextData& contextData) const;
	void ComputeSurf();
	void ComputePockets();
	const Molecule& GetMolecule() const;
	const Point& GetCenter();
	std::string GetName() const;
	DrawStyle GetDrawStyle() const;
	void SetDrawStyle(DrawStyle style);
	ColorStyle GetColorStyle() const;
	void SetColorStyle(ColorStyle useColor);
private:
	std::unique_ptr<Molecule> molecule;
	std::vector<std::unique_ptr<Vertex>> vertices;
	std::unordered_map<int, int> pockets;	// <Atom Serial, pocket ID>
	Point position;
	Rotation orientation;
	bool surfComputed;
	bool pocketsComputed;
	DrawStyle style;
	ColorStyle colorStyle;
	bool locked;	// currently locked by a dragger

	void DrawPoints(GLContextData& contextData) const;
	void DrawSurf(GLContextData& contextData) const;
	DrawMolecule::Color AtomColor(char short_name) const;
	DrawMolecule::Color AtomColor(int serial) const;
};

}

#endif /* DRAWMOLECULE_H_ */
