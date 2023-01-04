/*
 * Road.cpp
 *
 *  Created on: Dec 22, 2017
 *      Author: kg
 */

#include <iostream>
#include <math.h>
#include <set>
#include <cmath>
#include <math.h>
#include <stdexcept>

#include "../utility/Geometry.h"
#include "../parsed/Circle.h"
#include "../parsed/Intersections.h"
#include "Roads.h"
#include "Road.h"
#include "Section.h"

using namespace std;

Road::~Road() {
}

std::ostream& operator<<(std::ostream &strm, const Road &c) {
  return strm << "ROAD[name:" << c.name << "]";
}

//used for boost algorithms, separate from vertex, leaf, branch
void Road::BuildEdges(Roads& roads, Intersections& intersections)
{
  //find out how many unique intersection points in this circle, by unique, set of angles with connections
  // note that we cannot assume intersections will be only at 0 or 180, and when we do get other locations,
  // we have to take into account the weight starting at such location
  vector<Intersection> is = intersections.get();
  set<int> intersectionAngles;
  for (size_t i=0; i<is.size(); ++i) {
    if (name == is[i].GetCircleA()) {
      intersectionAngles.insert(is[i].GetAngleA());
    }
  }

  //NOTE that even though points in circle are interconnected, for the purpose of graphing, we will consider
  // them only connected to the intersection point, since that is the entry point, for the same reason,
  // when we start a route from such points we already should have the edge connecting to the intersection
  // point vertex

  //for each connected circle another edge is added weighing 0, because the distance is zero
  AddSectionsForIntersections(roads, intersections);
}

void Road::AddSectionsForIntersections(Roads& roads, Intersections& intersections)
{
  //by this time all vertices have been built, vertex count should remain the same
  //we just have to map those that are adjacent to another circle, thus intersections
  //we have to calculate what those numbers are, based on knowing the angles and the circle/road
  //indices,
  //no need to calculate weight, it is zero
  //find any intersections, create branches without intersections yet
  vector<Intersection> is = intersections.get();
  for (size_t i=0; i<is.size(); ++i) {
    if (name == is[i].GetCircleA()) {
      string nameB = is[i].GetCircleB();
      int angleA = is[i].GetAngleA();
      int angleB = is[i].GetAngleB();
      sections.push_back(Section(name, nameB, angleA, angleB, (double)0));
    }
  }
}
