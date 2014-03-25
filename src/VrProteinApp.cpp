/***********************************************************************
 VrProteinApp
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
#include "DrawMolecule.h"

using std::unique_ptr;

class VrProteinApp: public Vrui::Application {
public:
	VrProteinApp(int& argc, char**& argv);

	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void frame();

	unique_ptr<DrawMolecule> drawMolecule;
};


/******************************
 Methods of class VrProteinApp:
 ******************************/

VrProteinApp::VrProteinApp(int& argc, char**& argv) :
		Vrui::Application(argc, argv) {
	/* Set the navigation transformation to show the entire scene: */
	Vrui::setNavigationTransformation(Vrui::Point::origin, Vrui::Scalar(40));
}

void VrProteinApp::display(GLContextData& contextData) const {

	drawMolecule->Draw(contextData);
}

void VrProteinApp::frame() {
}


/* Create and execute an application object: */
int main(int argc, char* argv[]) {
	try {
		VrProteinApp app(argc, argv);
		// Load
		auto m = PDBImporter::ParsePDB("dna.pdb");
		app.drawMolecule = unique_ptr<DrawMolecule>(new DrawMolecule(move(m)));

		//init surf
		app.drawMolecule->ComputeSurf();
		app.drawMolecule->style = DrawStyle::Surf;

		app.run();
	} catch (std::runtime_error &err) {
		std::cerr << "Terminated program due to exception: " << err.what() << std::endl;
		return 1;
	}

	return 0;
}
