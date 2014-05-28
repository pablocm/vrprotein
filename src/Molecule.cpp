/*
 * Molecule.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#include <algorithm>
#include "Molecule.h"

using namespace std;

namespace VrProtein {

Molecule::Molecule() :
			center(Point::origin),
			center_calculated(false) {
}

void Molecule::AddAtom(unique_ptr<MolAtom> &atom) {
	atoms.push_back(move(atom));
	center_calculated = false;
}

const vector<unique_ptr<MolAtom>>& Molecule::GetAtoms() const {
	// http://stackoverflow.com/questions/1563124/c-vector-class-as-a-member-in-other-class
	// http://stackoverflow.com/questions/3777525/returning-a-unique-ptr-from-a-class-method-c0x
	return atoms;
}

const unique_ptr<MolAtom>& Molecule::FindBySerial(int serial) const {
	auto it = std::lower_bound(atoms.begin(), atoms.end(), serial,
			[](const unique_ptr<MolAtom>& a, const int serial) {
				return a->serial < serial;
			});

	const unique_ptr<MolAtom>& result = (it != atoms.end()) ? *it : atoms[0];

	// DEBUG
	if (result->serial != serial)
		throw std::runtime_error("FindBySerial failed.");
	return result;
}

const Point& Molecule::GetCenter() {
	if (!center_calculated) {
		Vector center_temp = Vector::zero;
		Scalar totalMass = 0;
		for (const auto& a : atoms) {
			center_temp += (a->position - Point::origin) * a->mass;
			totalMass += a->mass;
		}
		center_temp /= totalMass;
		center = Point(center_temp);
		center_calculated = true;
	}
	return center;
}

const Point& Molecule::GetCenter() const {
	if (!center_calculated)
		throw std::runtime_error("Called GetCenter() const when center is unknown.");
	return center;
}

}
