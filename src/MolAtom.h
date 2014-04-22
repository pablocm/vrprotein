/*
 * MolAtom.h
 *
 *  Created on: Mar 11, 2014
 *      Author: pablocm
 */

#ifndef MOLATOM_H_
#define MOLATOM_H_

#include <string>
#include "AffineSpace.h"

namespace VrProtein {

class MolAtom {
public:
	int serial;
	std::string name;
	std::string altLoc;
	std::string resName;
	std::string chainID;
	std::string resSeq;
	std::string iCode;
	Point position;
	Scalar occupancy;
	Scalar tempFactor;
	char short_name;	// H, C, N, O, F, S, etc...
	Scalar radius;
	Scalar mass;
};

}

#endif /* MOLATOM_H_ */
