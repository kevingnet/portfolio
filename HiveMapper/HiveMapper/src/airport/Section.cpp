/*
 * Section.cpp
 *
 *  Created on: Dec 24, 2017
 *      Author: kg
 */

#include <iostream>
#include <string>

#include "Section.h"

using namespace std;

Section::Section()
: roadA(""), roadB(""), pointA(0), pointB(0), weight(0.0)
{
}

Section::~Section() {
}

std::ostream& operator<<(std::ostream &strm, const Section &e) {
  return strm << "Section[" << e.roadA << ":" << e.pointA << "<->" << e.roadB
      << ":" << e.pointB << " weight:" << e.weight << "]";
}
