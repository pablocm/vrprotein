/*
 * Molecule.h
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */
#ifndef MOLECULE_H_
#define MOLECULE_H_

#include <memory>
#include <vector>
#include "MolAtom.h"

namespace VrProtein {

class Molecule {
public:
	std::string source_filename;	// archivo PDB

	Molecule();
	void AddAtom(std::unique_ptr<MolAtom> &atom);
	const std::vector<std::unique_ptr<MolAtom>>& GetAtoms() const;
	void GetCenter(float &x, float &y, float &z);
private:
	std::vector<std::unique_ptr<MolAtom>> atoms;
	float center_x;
	float center_y;
	float center_z;
	bool center_calculated;
};

}

#endif /* MOLECULE_H_ */
