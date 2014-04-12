/*
 * Molecule.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#include "Molecule.h"

using namespace std;

namespace VrProtein {

Molecule::Molecule() :
			center_x(0),
			center_y(0),
			center_z(0),
			center_calculated(false) {
}

void Molecule::AddAtom(unique_ptr<MolAtom> &atom) {
	atoms.push_back(move(atom));
}

const vector<unique_ptr<MolAtom>>& Molecule::GetAtoms() const {
	// http://stackoverflow.com/questions/1563124/c-vector-class-as-a-member-in-other-class
	// http://stackoverflow.com/questions/3777525/returning-a-unique-ptr-from-a-class-method-c0x
	return atoms;
}

void Molecule::GetCenter(float &x, float &y, float &z) {
	if (!center_calculated) {
		printf("Calculating atom center... ");
		for (auto& a : atoms) {
			center_x += a->x;
			center_y += a->y;
			center_z += a->z;
		}
		center_x /= atoms.size();
		center_y /= atoms.size();
		center_z /= atoms.size();
		center_calculated = true;
		printf("done.\n");
	}
	x = center_x;
	y = center_y;
	z = center_z;
}

}
