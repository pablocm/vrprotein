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

#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include "PDBImporter.h"

using namespace std;

class VrProteinRenderer: public Vrui::Application {
public:
	VrProteinRenderer(int& argc, char**& argv);

	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void frame();
};

/******************************
 Methods of class VrProteinRenderer:
 ******************************/

VrProteinRenderer::VrProteinRenderer(int& argc, char**& argv) :
		Vrui::Application(argc, argv) {
	/* Set the navigation transformation to show the entire scene: */
	Vrui::setNavigationTransformation(Vrui::Point::origin, Vrui::Scalar(12));
}

void VrProteinRenderer::display(GLContextData& contextData) const {
	/* Draw a red cube and a blue sphere: */
	glPushMatrix();

	glTranslated(-5.0, 0.0, 0.0);
	glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(1.0f, 0.5f, 0.0f));
	//glDrawCube(7.5f);
	glDrawWireframeCube(7.5f, 1.0f, 1.5f);

	glTranslated(10.0, 0.0, 0.0);
	glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT, GLColor<GLfloat, 4>(0.5f, 0.5f, 1.0f));
	glDrawSphereIcosahedron(4.5f, 6);

	glPopMatrix();
}

void VrProteinRenderer::frame() {
}

/* Create and execute an application object: */
int main(int argc, char* argv[]) {
	/*
	try {
		VrProteinRenderer app(argc, argv);
		app.run();
	} catch (std::runtime_error &err) {
		std::cerr << "Terminated program due to exception: " << err.what() << std::endl;
		return 1;
	}
	*/
	cout << "Start!" << endl;
	auto molecule = PDBImporter::ParsePDB("alanin.pdb");

	return 0;
}
