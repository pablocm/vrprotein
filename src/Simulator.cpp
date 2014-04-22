/*
 * Simulator.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: pablocm
 */

#include "Simulator.h"
#include "DrawMolecule.h"

namespace VrProtein {

Simulator::Simulator() {
	// TODO Auto-generated constructor stub
}

Simulator::SimResult Simulator::step(const DrawMolecule& ligand, const DrawMolecule& receptor) {
	auto result = SimResult();

	ONTransform ligTransform = ligand.GetState();
	ONTransform recTransform = receptor.GetState();

	for(const auto& ligAtom : ligand.GetMolecule().GetAtoms()) {
		for(const auto& recAtom : receptor.GetMolecule().GetAtoms()) {
			// Leonard-Jones potential:
			// V(r) = epsilon * ( (rm/r)^12 - 2*(rm/r)^6 )
			// rm := atom1.radius + atom2.radius  // distance at which the potential reaches minimum
			// epsilon := -1  // depth of the potential well
			Scalar rm = ligAtom->radius + recAtom->radius;
			Scalar r2 = Geometry::sqrDist(ligTransform.transform(ligAtom->position),
								recTransform.transform(recAtom->position));

			// trusting the compiler optimizations
			Scalar rm6 = rm * rm * rm * rm * rm * rm;
			Scalar r6 = r2 * r2 * r2;
			Scalar rm6_r6 = rm6 / r6;
			result.energy += -1 * (rm6_r6*rm6_r6 - 2*rm6_r6);
		}
	}

	return result;
}

}
