/*
 * Intersection.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: kg
 */

#include <iostream>

#include "Intersection.h"

using namespace std;

std::ostream& operator<<(std::ostream &strm, const Intersection &c) {
  return strm << "INTERSECTION[" << c.circleA << ":" << c.angleA
      << "x" << c.circleB << ":" << c.angleB << "]";
}
