/*
 * Simulator.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: pablocm
 */

#include <iostream>
#include "Simulator.h"
#include "DrawMolecule.h"

namespace VrProtein {

Simulator::Simulator() {
}

Simulator::SimResult Simulator::step(const DrawMolecule& ligand, const DrawMolecule& receptor,
		bool calcForces) {
	auto result = SimResult();
	ONTransform ligTransform = ligand.GetState();
	ONTransform recTransform = receptor.GetState();
	Point ligPos = ligand.GetPosition();

	// Find closest pocket (if receptor has any)


	for(const auto& ligAtom : ligand.GetMolecule().GetAtoms()) {
		Point ligAtomPos = ligTransform.transform(ligAtom->position);

		for(const auto& recAtom : receptor.GetMolecule().GetAtoms()) {
			Point recAtomPos = recTransform.transform(recAtom->position);
			// Leonard-Jones potential:
			// V(r) = epsilon * ( (rm/r)^12 - 2*(rm/r)^6 )
			// rm := atom1.radius + atom2.radius  // distance at which the potential reaches minimum
			// epsilon := -1  // depth of the potential well
			Scalar rm = ligAtom->radius + recAtom->radius;
			Scalar r2 = Geometry::sqrDist(ligAtomPos, recAtomPos);

			if (r2 > Math::sqr(2.806 * rm))	// truncate if dist greater than cut-off (r > 2.5*sigma)
				continue;

			// trusting the compiler optimizations
			Scalar rm6 = rm * rm * rm * rm * rm * rm;
			Scalar r6 = r2 * r2 * r2;
			Scalar rm6_r6 = rm6 / r6;
			Scalar energy = -1 * (rm6_r6*rm6_r6 - 2*rm6_r6);
			result.energy += energy;

			if (calcForces && energy != 0) {
				if (ligAtomPos != ligPos) { // Torque & force
					Vector r = ligAtomPos - ligPos; // position relative to center of mass
					Vector r_norm = r.normalize();
					Vector f = (recAtomPos - ligAtomPos).normalize() * energy; // force

					result.netTorque += Geometry::cross(r, f);
					result.netForce += (f * r_norm) * r_norm; // project force
				}
				else { // Force
					result.netForce += (recAtomPos - ligAtomPos).normalize() * energy; // force
				}
			}
		}
	}

	return result;
}

}
