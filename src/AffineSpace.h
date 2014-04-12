/*
 * AffineSpace.h
 *
 *  Created on: Apr 12, 2014
 *      Author: pablocm
 */

#ifndef AFFINESPACE_H_
#define AFFINESPACE_H_

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/Ray.h>

namespace VrProtein {

typedef double Scalar;
typedef Geometry::Vector<Scalar, 3> Vector;
typedef Geometry::Point<Scalar, 3> Point;
typedef Geometry::Rotation<Scalar, 3> Rotation;
typedef Geometry::OrthonormalTransformation<Scalar, 3> ONTransform;
typedef Geometry::Ray<Scalar, 3> Ray;

}

#endif /* AFFINESPACE_H_ */
