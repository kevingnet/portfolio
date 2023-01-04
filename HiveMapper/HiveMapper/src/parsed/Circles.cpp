/*
 * Circles.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: kg
 */

#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Circles.h"

using namespace std;

Circles::Circles(vector<string> circlesLines, bool debug) {
  size_t circleCount = boost::lexical_cast<size_t>(circlesLines[0]);

  if (circleCount + 1 != circlesLines.size()) {
    std::cout << "Invalid number of circles" << std::endl;
    throw out_of_range("Invalid number of circles");
  }

  vector<string> tokens;
  for (size_t i=1; i<circlesLines.size(); ++i) {
    boost::split(tokens, circlesLines[i], boost::is_any_of(" "));
    if (tokens.size() != 3) {
      std::cout << "Invalid number of tokens for circle" << std::endl;
      throw out_of_range("Invalid number of tokens for circle");
    }
    string name = tokens[0];
    int radius = boost::lexical_cast<int>(tokens[1]);
    if (radius < MINIMUM_RADIUS) {
      std::cout << "Invalid radius for circle, less than 1000" << std::endl;
      throw invalid_argument("Invalid radius for circle, less than 1000");
    }
    int intersections = boost::lexical_cast<int>(tokens[2]);
    circles.push_back(Circle(name, radius, intersections));
    tokens.clear();
  }
  if (debug)
    for (size_t i=0; i<circles.size(); ++i) {
      cout << circles[i] << endl;
    }
}
