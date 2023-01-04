/*
 * Path.cpp
 *
 *  Created on: Dec 27, 2017
 *      Author: kg
 */

#include <iostream>
#include <string>
#include "Path.h"

using namespace std;

Path::Path() {
}

Path::~Path() {
}

std::ostream& operator<<(std::ostream &strm, const Path &p) {
  for (size_t i=0; i<p.steps.size(); ++i) {
    strm << p.steps[i] << endl;
  }
  return strm;
}
