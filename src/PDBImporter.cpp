/*
 * PDBImporter.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include "utils/string.h"
#include "PDBImporter.h"

using namespace std;

namespace VrProtein {
namespace PDBImporter {

/**
 * Toma un archivo PDB y retorna la molecula.
 */
unique_ptr<Molecule> parsePDB(const string &filename) {
	cout << "Opening " << filename << endl;
	ifstream infile;
	infile.open(filename);
	if (infile.fail())
		throw std::runtime_error("file not found: " + filename);

	auto molecule = unique_ptr<Molecule>(new Molecule);
	molecule->source_filename = filename;

	string line;
	while (getline(infile, line)) {
		if (line.substr(0, 5) == "ATOM ") {
			auto atom = parsePDBAtom(line);
			molecule->addAtom(atom);
		}
	}
	infile.close();

	cout << "Loaded " << molecule->getAtoms().size() << " atoms." << endl;
	return molecule;
}

// http://www.wwpdb.org/documentation/format33/sect9.html#ATOM
/*
 COLUMNS        DATA  TYPE    FIELD        DEFINITION
 -------------------------------------------------------------------------------------
 1 -  6        Record name   "ATOM  "
 7 - 11        Integer       serial       Atom  serial number.
 13 - 16        Atom          name         Atom name.
 17             Character     altLoc       Alternate location indicator.
 18 - 20        Residue name  resName      Residue name.
 22             Character     chainID      Chain identifier.
 23 - 26        Integer       resSeq       Residue sequence number.
 27             AChar         iCode        Code for insertion of residues.
 31 - 38        Real(8.3)     x            Orthogonal coordinates for X in Angstroms.
 39 - 46        Real(8.3)     y            Orthogonal coordinates for Y in Angstroms.
 47 - 54        Real(8.3)     z            Orthogonal coordinates for Z in Angstroms.
 55 - 60        Real(6.2)     occupancy    Occupancy.
 61 - 66        Real(6.2)     tempFactor   Temperature  factor.
 77 - 78        LString(2)    element      Element symbol, right-justified.
 79 - 80        LString(2)    charge       Charge  on the atom.
 */
unique_ptr<MolAtom> parsePDBAtom(const string &line) {
	auto atom = unique_ptr<MolAtom>(new MolAtom);

	atom->serial = stoi(line.substr(6, 5));
	atom->name = trim(line.substr(12, 4));
	atom->altLoc = trim(line.substr(16, 1));
	atom->resName = trim(line.substr(17, 3));
	atom->position = Point(stof(line.substr(30, 8)), stof(line.substr(38, 8)),
			stof(line.substr(46, 8)));
	atom->occupancy = stof(line.substr(54, 6));
	atom->tempFactor = stof(line.substr(60, 6));
	atom->short_name = short_name(atom->name);
	atom->radius = default_radius(atom->short_name);
	atom->mass = default_mass(atom->short_name);

	//cout << atom->name << ": " << atom->x << ", " << atom->y << ", " << atom->z << endl;
	return atom;
}

// Nombre corto del atomo (H, C, N, O, F, S, etc.)
char short_name(const string& name) {
	const char *nm = name.c_str();
	// some names start with a number
	while (*nm && isdigit(*nm))
		nm++;
	return toupper(nm[0]);
}

// return a 'default' radius for a given atom name
// Taken from VMD: BaseMolecule.C:552
float default_radius(const char name) {
	float val;
	switch (name) {
	// These are similar to the values used by X-PLOR with sigma=0.8
	// see page 50 of the X-PLOR 3.1 manual
	case 'H':
		val = 1.00f;
		break;
	case 'C':
		val = 1.50f;
		break;
	case 'N':
		val = 1.40f;
		break;
	case 'O':
		val = 1.30f;
		break;
	case 'F':
		val = 1.20f;
		break;
	case 'S':
		val = 1.90f;
		break;
	default:
		val = 1.5f;
		break;
	}
	return val;
}

float default_mass(const char name) {
	float val;
	switch (name) {
	case 'H':
		val = 1.0f;
		break;
	case 'C':
		val = 12.0f;
		break;
	case 'N':
		val = 14.0f;
		break;
	case 'O':
		val = 16.0f;
		break;
	case 'F':
		val = 19.0f;
		break;
	case 'S':
		val = 32.0f;
		break;
	default:
		val = 12.0f;
		break;
	}
	return val;
}

}

}
