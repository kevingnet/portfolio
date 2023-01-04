/*
 * Step.cpp
 *
 *  Created on: Dec 27, 2017
 *      Author: kg
 */

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <cmath>
#include <algorithm>

#include "../utility/Geometry.h"
#include "Step.h"

using namespace std;

string format_double(float n) {
  stringstream stream;
  stream << fixed << setprecision(1) << n;
  return stream.str();
}

float round(float f) {
  return floor(10 * f + 0.5f) / 10;
}

Step::Step(Road* ra, Road* rb, int a, int b, bool ix, float w, bool dbg)
: roadSource(ra), roadDestination(rb), pointSource(a), pointDestination(b),
  intersection(ix), weight(w), debug(dbg), timeCost(0.0)
{
}

int Step::flipDirection(int direction) {
  int dir = direction;
  if(dir == CW)
    dir = CCW;
  else
    dir = CW;
  return dir;
}

// two operations can cause a shift in direction
// a) a transfer into an 'outside' circle, that is when the two angles oppose by 180
// b) the drone decides it's faster to shift when the angle to reach is over 180
bool Step::reverseByTransfer() {
  if (intersection) {
    int distance = angular_distance(pointSource, pointDestination);
    return distance == 180 ? true : false;
  }
  return false;
}

bool Step::reverseByDistanceAndDirection(int direction) {
  if (!intersection) {
    int distance = angular_distance(pointSource, pointDestination);
    //per calculations, it is worth it to shift direction when the distance
    //exceeds 180
    if (distance == 180)
      return false;
    int dst = circular_angle_counter(pointDestination - pointSource);
    //if distance < 180, direction should be CW, else CCW
    int neededDirection = -1;
    if (dst < 180)
      neededDirection = distance < 180 ? CW : CCW;
    else
      neededDirection = distance < 180 ? CCW : CW;
    return direction != neededDirection;
  }
  return false;
}

//directionality is proving a bit tricky given this inelegant code, if time allows
//TODO: encapsulate all these data and code into more specific objects, perhaps an FSM
//When a TRANSFER happens and if it causes a REVERSE, the operation is not to be
//performed right then and there, but rather, it would be a directive for the next step,
//the directive travels in the form of altering the direction. the next step will receive it
//NOTE: for first step, we'll assume speed is zero, therefore, extra calculations are needed
int Step::CalculateCommand(int direction, bool& started, int step)
{
  if (debug)
    cout << "\t\t\tstep: " << step << endl;

  int enteredDirection = direction;
  if (intersection) {
    if (reverseByTransfer()) {
      //that's all we do here with direction, next step will pick up
      direction = flipDirection(direction);
    }
    //for steps that involve two roads, there can only be one command, transfer
    //transfer takes 0.1 seconds
    command = "TRANSFER " + roadDestination->getName() + " (" + format_double(TIME_TRANSFER) + ")\n";
    timeCost += TIME_TRANSFER;
    printTransfer(enteredDirection, direction);
    return direction;
  }

  //based on goal destination, determine if a reverse operation is needed
  //if goal > 180, it is optimal to shift, even considering smaller circles, the smallest
  //is 1000 radius and even for that it is worthwhile, as for smaller circles it would be
  //adventageous to calculate optimal dirertion, however in this case all circles will
  //be >= 1000 radius
  //or course we must also consider the direction, direction has already been shifted as
  //necessary, so what we get now is what we work with
  //we must also consider angle offset when angles aren't zero,
  //if goal is a distance of 180 as we said before, no reverse is needed
  //if we are going CW (direction 0(false))
  //find out based on current direction if we should reverse
  reversed = false;
  float goTime = 0.0;
  //when we calculate go time, we must account for distance lapsed during acceleration
  //this is not so when reversing because accel=decel so we endup where we started
  if (reverseByDistanceAndDirection(direction)) {
    direction = flipDirection(direction);
    //the time it takes to stop accelerate to 4m/s 8.0 seconds 4 to stop and 4 to accel.
    if (started) {
      command += "STOP (" + format_double(TIME_DECELERATE) + ")\n";
      timeCost += TIME_DECELERATE;
    }
    //shift direction takes 0.3 seconds, plus
    command += "REVERSE (" + format_double(TIME_REVERSE) + ")\n";
    timeCost += TIME_REVERSE;
    reversed = true;
    if (started) {
      //added to the GO command below, this is the cost to accelerate to full speed
      //to contrarest the previous stop operation which also took 4 seconds, as far as distance
      //it's not affected since we end up where we started
      timeCost += TIME_ACCELERATE;
      started = true;
      if (debug)
        cout << "START* at reverse\n";
    }
  }
  bool hadStarted = started;
  goTime = round(weight / SPEED_MAX_MPS);
  if (!started) {
    //we subtract from weight because this is distance traveled during the
    //time to accelerate
    goTime += TIME_ACCELERATE;
    started = true;
    if (debug)
      cout << "START*\n";
  }
  timeCost += round(goTime);
  command += "GO (" + format_double(goTime) + ")\n";
  printDisplacement(enteredDirection, direction, hadStarted, goTime);
  return direction;
}

int Step::CalculateCommandFast(int direction, bool& started, int step)
{
  int enteredDirection = direction;
  if (intersection) {
    if (reverseByTransfer()) {
      direction = flipDirection(direction);
    }
    timeCost += TIME_TRANSFER;
    return direction;
  }
  reversed = false;
  float goTime = 0.0;
  if (reverseByDistanceAndDirection(direction)) {
    direction = flipDirection(direction);
    if (started) {
      timeCost += TIME_DECELERATE;
    }
    timeCost += TIME_REVERSE;
    reversed = true;
    if (started) {
      timeCost += TIME_ACCELERATE;
      started = true;
    }
  }
  bool hadStarted = started;
  goTime = round(weight / SPEED_MAX_MPS);
  if (!started) {
    goTime += TIME_ACCELERATE;
    started = true;
  }
  timeCost += round(goTime);
  return direction;
}

Step::~Step() {
}

void Step::printTransfer(int enteredDirection, int direction) {
  if (debug) {
    cout << roadSource->getName() << pointSource;
    cout << "->" << roadDestination->getName() << pointDestination;
    if (enteredDirection == direction) {
      if (direction == CW)
        cout << "  CW" << endl;
      else
        cout << "  CCW" << endl;
    } else {
      if (enteredDirection == CW)
        cout << " REVERSE CW";
      else
        cout << " REVERSE CCW";
      if (direction == CW)
        cout << "->CW" << endl;
      else
        cout << "->CCW" << endl;
    }
    cout << "COMMAND: \n" << command << endl;
    command += "\n";
  }
}

void Step::printDisplacement(int enteredDirection, int direction, bool started, float goTime) {
  if (debug) {
    cout << roadSource->getName() << pointSource;
    cout << "->" << roadDestination->getName() << pointDestination;
    if (reversed == true) {
      if (started)
        cout << " STOP REVERSE";
      else
        cout << " REVERSE";
    }
    cout << " GO: " << fixed << setprecision(1) << goTime;
    if (enteredDirection == direction) {
      if (direction == CW)
        cout << "  CW" << endl;
      else
        cout << "  CCW" << endl;
    } else {
      if (enteredDirection == CW)
        cout << " REVERSE CW";
      else
        cout << " REVERSE CCW";
      if (direction == CW)
        cout << "->CW" << endl;
      else
        cout << "->CCW" << endl;
    }
    cout << "COMMAND: \n" << command << endl;
    command += "\n";
  }
}

std::ostream& operator<<(std::ostream &strm, const Step &s) {
  if (s.intersection)
    return strm << "Step Intersection[" << s.roadSource->getName() << ":" << s.pointSource
        << " -> " << s.roadDestination->getName() << ":" << s.pointDestination << "]";
  else
    return strm << "Step Displacement[" << s.roadSource->getName() << ":" << s.pointSource
        << " -> " << s.roadDestination->getName() << ":" << s.pointDestination
        << " weight:" << s.weight << "]";
}
