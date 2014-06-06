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
		bool calcForces) const {
	auto result = SimResult();
	ONTransform ligTransform = ligand.GetState();
	ONTransform recTransform = receptor.GetState();
	Point ligPos = ligTransform.getOrigin();

	// Calculate overlapping
	result.overlappingAmount = ligand.Intersects(receptor);

	// Find closest pocket to ligand (if receptor has any)
	result.closestPocket = -1;
	Scalar closestPocketDist2 = 999; // max valid distance^2
	for (const auto& it : receptor.GetPocketCentroids()) {
		auto d2 = (ligPos - recTransform.transform(it.second)).sqr();
		if (d2 < closestPocketDist2) {
			closestPocketDist2 = d2;
			result.closestPocket = it.first;
		}
	}
	// Calculate mean pocket distance
	if (result.closestPocket != -1) {
		auto pocketSpheres = receptor.GetSpheresOfPocket(result.closestPocket);
		for (const auto& ligAtom : ligand.GetMolecule().GetAtoms()) {
			// find closest sphere
			Scalar minDist2 = 999;
			for (const auto& sphere : pocketSpheres) {
				auto d2 = (ligTransform.transform(ligAtom->position) - recTransform.transform(sphere.getCenter())).sqr();
				if (d2 < minDist2)
					minDist2 = d2;
			}
			result.meanPocketDist += minDist2;
		}
		result.meanPocketDist = Math::sqrt(result.meanPocketDist) / pocketSpheres.size();
	}
	else
		result.meanPocketDist = 999;


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

bool Simulator::compare(const Simulator::SimResult& sr1, const Simulator::SimResult& sr2) const {
	return sr1.overlappingAmount < 2.0 && sr1.meanPocketDist <= sr2.meanPocketDist;
}

}
