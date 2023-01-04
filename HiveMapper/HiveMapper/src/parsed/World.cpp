/*
 * World.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: kg
 */

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "World.h"
#include "../airport/Roads.h"

using namespace std;

World::World(vector<string> c,
    vector<string> i,
    vector<string> r,
    bool d)
  : circles(c, d), intersections(i, d), planner(&roads, d), debug(d) {
  SetRoute(r);
}

void World::SetRoute(vector<string> routeData) {
  if (routeData.size() != 2) {
    std::cout << "Invalid number of lines for route, should be exactly two" << std::endl;
    throw out_of_range("Invalid number of lines for route, should be exactly two");
  }

  vector<string> tokens;
  boost::split(tokens, routeData[0], boost::is_any_of(" "));
  if (tokens.size() != 3) {
    std::cout << "Invalid number of tokens for route origin" << std::endl;
    throw out_of_range("Invalid number of tokens for route origin");
  }

  string circleOrigin = tokens[0];
  int locationOrigin = boost::lexical_cast<int>(tokens[1]);
  string direction = tokens[2];
  tokens.clear();

  boost::split(tokens, routeData[1], boost::is_any_of(" "));
  if (tokens.size() != 2) {
    std::cout << "Invalid number of tokens for route destination" << std::endl;
    throw out_of_range("Invalid number of tokens for route destination");
  }

  string circleDestination = tokens[0];
  int locationDestination = boost::lexical_cast<int>(tokens[1]);
  tokens.clear();

  int dir = -1;
  if (direction == "+") {
    dir = Route::CLOCK_WISE;
  } else if (direction == "-") {
    dir = Route::COUNTER_CLOCK_WISE;
  } else {
    std::cout << "Invalid direction for route, character is invalid" << std::endl;
    throw invalid_argument("Invalid direction for route, character is invalid");
  }

  route = Route(circleOrigin, circleDestination, locationOrigin, locationDestination, dir);
  if (debug)
    cout << route << endl << endl;

  if (debug)
    cout << "BUILDING GRAPH ELEMENTS..." << endl;
  roads.Build(circles);
  roads.BuildEdges(intersections);
}

void World::Solve(string algorithm) {
  int al = Planner::EXHAUSTIVE;
  if (algorithm == "exhaustive") {
    al = Planner::EXHAUSTIVE;
  } else if (algorithm == "bfs") {
    al = Planner::BFS;
  } else if (algorithm == "dijkstra") {
    al = Planner::DIJKSTRA;
  }
  planner.Solve(route, al);
}
