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
#include "Molecule.h"
#include "MolAtom.h"

using namespace std;

namespace PDBImporter {

/**
 * Toma un archivo PDB y retorna la molecula.
 */
unique_ptr<Molecule> ParsePDB(const string &filename) {
	cout << "Opening " << filename << endl;
	ifstream infile;
	infile.open(filename);
	if (infile.fail())
		throw std::runtime_error("file not found");

	auto molecule = unique_ptr<Molecule>(new Molecule);
	string line;
	while (getline(infile, line)) {
		if (line.substr(0, 5) == "ATOM ") {
			auto atom = ParsePDBAtom(line);
			molecule->AddAtom(atom);
		}
	}
	infile.close();

	cout << "Loaded " << molecule->GetAtoms().size() << " atoms." << endl;
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
unique_ptr<MolAtom> ParsePDBAtom(const string &line) {
	auto atom = unique_ptr<MolAtom>(new MolAtom);

	atom->serial 	= stoi(line.substr(6, 5));
	atom->name 		= trim(line.substr(12, 4));
	atom->altLoc 	= trim(line.substr(16, 1));
	atom->resName 	= trim(line.substr(17, 3));
	atom->x 		= stof(line.substr(30, 8));
	atom->y 		= stof(line.substr(38, 8));
	atom->z 		= stof(line.substr(46, 8));
	atom->occupancy = stof(line.substr(54, 6));
	atom->tempFactor= stof(line.substr(60, 6));

	//cout << atom->name << ": " << atom->x << ", " << atom->y << ", " << atom->z << endl;
	return atom;
}
}
