/*
 * MolAtom.h
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#ifndef MOLATOM_H_
#define MOLATOM_H_

#include <string>

class MolAtom {
public:
	int serial;
	std::string name;
	std::string altLoc;
	std::string resName;
	std::string chainID;
	std::string resSeq;
	std::string iCode;
	float x;
	float y;
	float z;
	float occupancy;
	float tempFactor;
};

#endif /* MOLATOM_H_ */
