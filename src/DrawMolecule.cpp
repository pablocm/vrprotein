/*
 * DrawMolecule.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: pablocm
 */

#include <memory>
#include "DrawMolecule.h"
#include "Molecule.h"

using namespace std;

DrawMolecule::DrawMolecule(unique_ptr<Molecule> m) {
	molecule = move(m);
}

/**
 * Ver:
 * DrawMolItemSurface.C:42 (draw_surface)
 * Surf.C:35 (compute)
 *
 * Para parsear output:
 * http://www.cplusplus.com/reference/cstdio/fscanf/
 */
