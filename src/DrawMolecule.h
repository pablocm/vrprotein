/*
 * DrawMolecule.h
 *
 *  Created on: Mar 16, 2014
 *      Author: pablocm
 */

#ifndef DRAWMOLECULE_H_
#define DRAWMOLECULE_H_

#include <memory>
#include "Molecule.h"

class DrawMolecule {
public:
	DrawMolecule(std::unique_ptr<Molecule> m);

private:
	std::unique_ptr<Molecule> molecule;
};

#endif /* DRAWMOLECULE_H_ */
