/***********************************************************************
 VrProteinRenderer
 Copyright (c) 2014 Pablo Cruz

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ***********************************************************************/

#include <memory>
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include "PDBImporter.h"
#include "Molecule.h"

using std::unique_ptr;

class VrProteinRenderer: public Vrui::Application {
public:
	VrProteinRenderer(int& argc, char**& argv);

	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void frame();

	unique_ptr<Molecule> molecule;
};

// TEMP TEMP TEMP
void DrawMolecule(Molecule& molecule) {

	glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(0.5f, 0.5f, 1.0f));
	glDrawSphereIcosahedron(0.1f, 3);

	glPointSize(2.0f);

	//glBegin(GL_POINTS);
	for (auto& a : molecule.GetAtoms()) {
		if (a->name.substr(0, 1) == "C")
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(0.0f, 1.0f, 0.0f));
		else if (a->name.substr(0, 1) == "H")
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(1.0f, 0.0f, 0.0f));
		else if (a->name.substr(0, 1) == "O")
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(0.2f, 0.2f, 1.0f));
		else
			glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(1.0f, 1.0f, 0.0f));

		//glVertex3f(a->x, a->y, a->z);

		glPushMatrix();
		glTranslated(a->x, a->y, a->z);
		glDrawSphereIcosahedron(0.1f, 1);
		glPopMatrix();
	}
	//glEnd();
}

/******************************
 Methods of class VrProteinRenderer:
 ******************************/

VrProteinRenderer::VrProteinRenderer(int& argc, char**& argv) :
		Vrui::Application(argc, argv) {
	/* Set the navigation transformation to show the entire scene: */
	Vrui::setNavigationTransformation(Vrui::Point::origin, Vrui::Scalar(40));
}

void VrProteinRenderer::display(GLContextData& contextData) const {
	//glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_LINE_BIT|GL_POINT_BIT);
	//glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

	// TEMP
	DrawMolecule(*molecule);

	//glPopAttrib();

//	/* Draw a red cube and a blue sphere: */
//	glPushMatrix();
//
//	glTranslated(-5.0, 0.0, 0.0);
//	glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(1.0f, 0.5f, 0.0f));
//	//glDrawCube(7.5f);
//	glDrawWireframeCube(7.5f, 1.0f, 1.5f);
//
//	glTranslated(10.0, 0.0, 0.0);
//	glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(0.5f, 0.5f, 1.0f));
//	glDrawSphereIcosahedron(4.5f, 6);
//
//	glPopMatrix();
}

void VrProteinRenderer::frame() {
}

/* Create and execute an application object: */
int main(int argc, char* argv[]) {
	try {
		VrProteinRenderer app(argc, argv);
		// Load
		app.molecule = PDBImporter::ParsePDB("dna.pdb");

		app.run();
	} catch (std::runtime_error &err) {
		std::cerr << "Terminated program due to exception: " << err.what() << std::endl;
		return 1;
	}

	return 0;
}
