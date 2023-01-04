/*
 * Intersections.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: kg
 */
#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Intersections.h"

using namespace std;

Intersections::Intersections(vector<string> intersectionLines, bool debug) {
  size_t intersectionCount = boost::lexical_cast<size_t>(intersectionLines[0]);

  if (intersectionCount + 1 != intersectionLines.size()) {
    std::cout << "Invalid number of intersections" << std::endl;
    throw out_of_range("Invalid number of intersections");
  }

  vector<string> tokens;
  for (size_t i=1; i<intersectionLines.size(); ++i) {
    boost::split(tokens, intersectionLines[i], boost::is_any_of(" "));
    if (tokens.size() != 4) {
      std::cout << "Invalid number of tokens for intersection" << std::endl;
      throw out_of_range("Invalid number of tokens for intersection");
    }

    string nameA = tokens[0];
    string nameB = tokens[2];
    const int angleA = boost::lexical_cast<int>(tokens[1]);;
    const int angleB = boost::lexical_cast<int>(tokens[3]);;
    intersections.push_back(Intersection(nameA, nameB, angleA, angleB));
    tokens.clear();
  }
  if (debug)
    for (size_t i=0; i<intersections.size(); ++i) {
      cout << intersections[i] << endl;
    }
}
