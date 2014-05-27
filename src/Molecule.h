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
#include "AffineSpace.h"
#include "MolAtom.h"

namespace VrProtein {

class Molecule {
public:
	std::string source_filename;	// archivo PDB

	Molecule();
	void AddAtom(std::unique_ptr<MolAtom> &atom);
	const std::vector<std::unique_ptr<MolAtom>>& GetAtoms() const;
	const std::unique_ptr<MolAtom>& FindBySerial(int serial) const;
	const Point& GetCenter();
private:
	std::vector<std::unique_ptr<MolAtom>> atoms;
	Point center;
	bool center_calculated;
};

}

#endif /* MOLECULE_H_ */
