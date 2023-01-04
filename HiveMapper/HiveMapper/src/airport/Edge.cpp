/*
 * Edge.cpp
 *
 *  Created on: Jan 4, 2018
 *      Author: kg
 */

#include <iostream>

#include "Edge.h"

Edge::Edge() {
}

Edge::~Edge() {
}

std::ostream& operator<<(std::ostream &strm, const Edge &c) {
  return strm << "Edge[name1:" << c.name1 << " name2:"<< c.name2 << "]";
}

