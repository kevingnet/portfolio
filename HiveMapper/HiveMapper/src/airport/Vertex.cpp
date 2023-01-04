/*
 * Vertex.cpp
 *
 *  Created on: Jan 4, 2018
 *      Author: kg
 */

#include <iostream>
#include "Vertex.h"

Vertex::Vertex() {
}

Vertex::~Vertex() {
}

std::ostream& operator<<(std::ostream &strm, const Vertex &c) {
  return strm << "Vertex[name:" << c.name << "]";
}
