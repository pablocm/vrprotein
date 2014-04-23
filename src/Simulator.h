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
		Vector netForce;
		Vector netTorque;
		Scalar energy;
	};

	/* Methods */
	Simulator();
	SimResult step(const DrawMolecule& ligand, const DrawMolecule& receptor, bool calcForces);
};

}

#endif /* SIMULATOR_H_ */
