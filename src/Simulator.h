/*
 * Simulator.h
 *
 *  Created on: Apr 22, 2014
 *      Author: pablocm
 */

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

//#include <memory>
//#include <vector>
#include "AffineSpace.h"

namespace VrProtein {

/* Forward declarations: */
class DrawMolecule;

class Simulator {
public:
	/* Embedded classes: */
	struct SimResult {
		Scalar overlappingAmount;
		Vector netForce;
		Vector netTorque;
		Scalar energy;
		int closestPocket;
		Scalar meanPocketDist;
	};

	/* Methods */
	Simulator();
	SimResult step(const DrawMolecule& ligand, const DrawMolecule& receptor, bool calcForces) const;
	bool compare(const SimResult& sr1, const SimResult& sr2) const;	// True if sr1 is better one
};

}

#endif /* SIMULATOR_H_ */
