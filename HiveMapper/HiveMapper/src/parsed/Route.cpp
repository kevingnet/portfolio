/*
 * Route.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: kg
 */

#include <iostream>
#include <cmath>

#include "Route.h"

using namespace std;

std::ostream& operator<<(std::ostream &strm, const Route &r) {
  return strm << "ROUTE[origin:" << r.circleDeparture << " angle:" << r.angleDeparture << " direction:" << r.direction
              << " destination:" << r.circleArrival << " angle:" << r.angleArrival << "]";
}
