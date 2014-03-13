/*
 * Molecule.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#include <memory>
//#include <vector>
#include "Molecule.h"
#include "MolAtom.h"

using namespace std;

Molecule::Molecule() {
	// TODO Auto-generated constructor stub
}

void Molecule::AddAtom(unique_ptr<MolAtom> &atom) {
	atoms.push_back(move(atom));
}

MolAtom& Molecule::GetAtom() {
	// http://stackoverflow.com/questions/3777525/returning-a-unique-ptr-from-a-class-method-c0x
	MolAtom& atom = *atoms[0];
	return atom;
}

int Molecule::AtomCount() {
	return atoms.size();
}
