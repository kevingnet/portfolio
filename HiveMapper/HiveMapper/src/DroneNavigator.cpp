//============================================================================
// Name        : DroneNavigator.cpp
// Author      : Kevin Guerra
// Version     :
//     :
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <string>
#include <vector>

#include "utility/ParseInputFile.h"
#include "utility/ProcessCommandLine.h"
#include "parsed/World.h"

using namespace std;

int main(int argc, char** argv) {
  string algorithm = GetCommandLineOption(argv, argv + argc, "-a", "dijkstra");
  string inputFileName = GetCommandLineOption(argv, argv + argc, "-f", "./DroneNavigator.world");
  string outputDebug = GetCommandLineOption(argv, argv + argc, "-d");
  string outputHelp = GetCommandLineOption(argv, argv + argc, "-h");

  if (!outputHelp.empty()) {
    cout << "DroneNavigator v0.1" << endl
        << "-d\t- Output extra information." << endl
        << "-a\t- Select algorithm to use. dijkstra, bfs or exhaustive" << endl
        << "-f\t- Input file with roads, intersections and route descriptions. Default: 'DroneNavigator.world'" << endl;
    return 0;
  }

  bool debug = outputDebug != "";
  if (debug) {
    cout << "Using input file" << inputFileName  << endl;
    cout << "Using algorithm " << algorithm << endl;
  }

  vector<string> circles;
  vector<string> intersections;
  vector<string> route;

  if (debug)
    cout << "PARSING INPUT FILE..." << endl;
  if (!ParseInputFile(inputFileName, circles, intersections, route)) {
    cout << "Error parsing input file" << endl;
  }

  try {
    World world(circles, intersections, route, debug);
    world.Solve(algorithm);
  } catch(const exception& e) {
    cout << "Exception " << e.what() << endl;
    return 1;
  }

	return 0;
}
