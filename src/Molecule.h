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
	void addAtom(std::unique_ptr<MolAtom> &atom);
	const std::vector<std::unique_ptr<MolAtom>>& getAtoms() const;
	const std::unique_ptr<MolAtom>& findBySerial(int serial) const;
	void fixCenterOffset();	// Moves all atoms so that the centroid is at the origin
	const Vector& getOffset() const;	// Get offset (used to displace surf mesh)
private:
	std::vector<std::unique_ptr<MolAtom>> atoms;
	Vector offset;
};

}

#endif /* MOLECULE_H_ */
