/*
 * Geometry.cpp
 *
 *  Created on: Dec 28, 2017
 *      Author: kg
 */

#include <cmath>

#include "Geometry.h"

using namespace std;

int angular_distance(int first, int second) {
  int raw_diff = first > second ? first - second : second - first;
  int mod_diff = fmod(raw_diff, 360.0);
  int dist = mod_diff > 180.0 ? 360.0 - mod_diff : mod_diff;
  return dist;
}

int circular_angle_counter(int i) {
  while (i < 0) { i += 360; }
  while (i >= 360) { i -= 360; }
  return i;
}




