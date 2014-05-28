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
	void FixCenterOffset();	// Moves all atoms so that the centroid is at the origin
	const Vector& GetOffset() const;	// Get offset (used to displace surf mesh)
private:
	std::vector<std::unique_ptr<MolAtom>> atoms;
	Vector offset;
};

}

#endif /* MOLECULE_H_ */
