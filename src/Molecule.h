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

class Molecule {
public:
	Molecule();
	void AddAtom(std::unique_ptr<MolAtom> &atom);
	const std::vector<std::unique_ptr<MolAtom>>& GetAtoms() const;
	float default_radius(char *nm);
private:
	std::vector<std::unique_ptr<MolAtom>> atoms;
};

#endif /* MOLECULE_H_ */
