/*
 * Molecule.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#include <algorithm>
#include <string>
#include "Molecule.h"

using namespace std;

namespace VrProtein {

Molecule::Molecule() {
}

void Molecule::AddAtom(unique_ptr<MolAtom> &atom) {
	atoms.push_back(move(atom));
}

const vector<unique_ptr<MolAtom>>& Molecule::GetAtoms() const {
	// http://stackoverflow.com/questions/1563124/c-vector-class-as-a-member-in-other-class
	// http://stackoverflow.com/questions/3777525/returning-a-unique-ptr-from-a-class-method-c0x
	return atoms;
}

const unique_ptr<MolAtom>& Molecule::FindBySerial(int serial) const {
	// Assumes atoms were inserted in serial order
	auto it = std::lower_bound(atoms.begin(), atoms.end(), serial,
			[](const unique_ptr<MolAtom>& a, const int s) {
				return a->serial < s;
			});

	const unique_ptr<MolAtom>& result = (it != atoms.end()) ? *it : atoms[0];

	// DEBUG
	if (result->serial != serial)
		throw std::runtime_error(std::string("FindBySerial failed for: ") + std::to_string(serial));
	return result;
}

void Molecule::FixCenterOffset() {
	// Calculate center (and offset)
	Vector center = Vector::zero;
	Scalar totalMass = 0;
	for (const auto& a : atoms) {
		center += (a->position - Point::origin) * a->mass;
		totalMass += a->mass;
	}
	center /= totalMass;
	offset = -center;

	// Translate atoms
	for (auto& a : atoms) {
		a->position += offset;
	}
}

const Vector& Molecule::GetOffset() const {
	return offset;
}

}
