/*
 * Circle.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: kg
 */

#include <iostream>

#include "Circle.h"
#include <boost/lexical_cast.hpp>

using namespace std;

Circle::Circle(const string name, const int radius, const int intersections) :
  name(name), radius(radius), intersections(intersections)
{
}

bool Circle::operator==(const Circle& c) const
{
  return name == c.name;
}

std::ostream& operator<<(std::ostream &strm, const Circle &c) {
  return strm << "CIRCLE[name:" << c.name << " radius:" << c.radius
      << " intersections:" << c.intersections << "]";
}

