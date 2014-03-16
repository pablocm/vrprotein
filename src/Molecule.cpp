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

const vector<unique_ptr<MolAtom>>& Molecule::GetAtoms() const {
	// http://stackoverflow.com/questions/1563124/c-vector-class-as-a-member-in-other-class
	// http://stackoverflow.com/questions/3777525/returning-a-unique-ptr-from-a-class-method-c0x
	return atoms;
}

// return a 'default' radius for a given atom name
// Taken from VMD: BaseMolecule.C:552
float Molecule::default_radius(char *nm) {
  float val = 1.5f;
  // some names start with a number
  while (*nm && isdigit(*nm))
    nm++;
  if(nm) {
    switch(toupper(nm[0])) {
      // These are similar to the values used by X-PLOR with sigma=0.8
      // see page 50 of the X-PLOR 3.1 manual
      case 'H' : val = 1.00f; break;
      case 'C' : val = 1.50f; break;
      case 'N' : val = 1.40f; break;
      case 'O' : val = 1.30f; break;
      case 'F' : val = 1.20f; break;
      case 'S' : val = 1.90f; break;
    }
  }

  return val;
}
