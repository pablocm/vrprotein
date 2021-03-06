/*
 * PDBImporter.h
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#ifndef PDBIMPORTER_H_
#define PDBIMPORTER_H_

#include <memory>
#include "Molecule.h"

namespace VrProtein {
namespace PDBImporter {
/**
 * Toma un archivo PDB y retorna la molecula.
 */
std::unique_ptr<Molecule> parsePDB(const std::string &filename);
std::unique_ptr<MolAtom> parsePDBAtom(const std::string &line);
char short_name(const std::string& name);
float default_radius(const char name);
float default_mass(const char name);
}

}

#endif /* PDBIMPORTER_H_ */
