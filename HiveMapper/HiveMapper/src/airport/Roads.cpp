/*
 * Roads.cpp
 *
 *  Created on: Dec 22, 2017
 *      Author: kg
 */

#include "Roads.h"
#include "Road.h"

using namespace std;

Roads::~Roads() {
}

Road* Roads::getByName(string name) {
  for (size_t i=0; i<roads.size(); ++i) {
    if (name == roads[i].getName()) {
      return &roads[i];
    }
  }
  return 0;
}

void Roads::Build(Circles& circles) {
  vector<Circle>& cs = circles.getCircles();
  for (size_t i=0; i<cs.size(); ++i) {
    roads.push_back(Road(cs[i].getName(), cs[i].getRadius()));
  }
}

void Roads::BuildEdges(Intersections& intersections) {
  for (size_t i=0; i<roads.size(); ++i) {
    roads[i].BuildEdges(*this, intersections);
  }
}
