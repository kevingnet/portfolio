/*
 * Planner.cpp
 *
 *  Created on: Dec 23, 2017
 *      Author: kg
 */

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <utility>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <stdlib.h>
#include <ctime>
#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
//#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>

#include "../utility/Geometry.h"
#include "GraphElementsMaps.h"
#include "Planner.h"
#include "Path.h"
#include "Section.h"
#include "Step.h"
#include "Graph.h"

using namespace boost;
namespace bpt = boost::posix_time;
using namespace bpt;

Planner::~Planner() {
}

void Planner::Solve(Route& route, int algorithm) {
  switch (algorithm) {
  case EXHAUSTIVE:
    SolveExhaustive(route);
    break;
  case BFS:
    SolveBFS(route);
    break;
  case DIJKSTRA:
    SolveDijkstra(route);
    break;
  }
}

bool vectorSizeCompare(const std::vector<std::pair<int, int> >& a,
    const std::vector<std::pair<int, int> >& b) {
  return a.size() < b.size();
}

bool profileTimesCompare(const std::pair<float, int>& a,
    const std::pair<float, int>& b) {
  return a.first < b.first;
}

double calculate_weight(double segmentSize, int angleA, int angleB) {
  int angleDistance = angular_distance(angleA, angleB);
  return angleDistance * segmentSize;
}
//==============================================================================

class custom_bfs_visitor : public default_bfs_visitor
{
public:

  template < typename VertexDescriptor, typename t_graph >
  void discover_vertex(VertexDescriptor u, const t_graph & g) const
  {
    std::cout << u << std::endl;
  }
};

//==============================================================================
// EXHAUSTIVE
//==============================================================================
void Planner::SolveExhaustive(Route& route) {
  start = bpt::microsec_clock::local_time();
  if (debug)
    std::cout << std::endl
        << "Calculating route using 'Exhaustive' algorithm for: " << route
        << std::endl;

  const std::string routeNameDeparture = route.getCircleDeparture();
  const std::string routeNameArrival = route.getCircleArrival();

  const std::string srcVertexName = routeNameDeparture
      + lexical_cast<std::string>(route.getAngleDeparture());
  const std::string dstVertexName = routeNameArrival
      + lexical_cast<std::string>(route.getAngleArrival());

  vertexDeptarture = Vertex(routeNameDeparture, srcVertexName, route.getAngleDeparture());
  vertexArrival = Vertex(routeNameArrival, dstVertexName, route.getAngleArrival());

  if (routeNameDeparture == routeNameArrival) {
    std::cout << "ERROR! Departure is the same as arrival\n";
    return;
  }

  FillVertices();
  int graphSize = vertices.size();
  int solutionIndex = 0;

  Graph g(graphSize);

  for (std::map<std::string, Vertex>::iterator it=vertices.begin(); it!=vertices.end(); ++it) {
    int v = g.addVertex(it->first);
    map.addItem(it->second.getRoadName(), it->first, v, it->second.getAngle());
  }

  FillEdges();
  for (std::map<std::string, Edge>::iterator it=edges.begin(); it!=edges.end(); ++it) {
    g.addEdge(it->second.getName1(), it->second.getName2(), it->second.getWeight());
  }

  const int srcVertex1 = map.getNameVertex(srcVertexName);
  const int dstVertex1 = map.getNameVertex(dstVertexName);

  g.allPaths(srcVertex1, dstVertex1);

  std::vector<std::vector<std::pair<int, int> > >& allPaths = g.getAllPaths();

  std::sort(allPaths.begin(), allPaths.end(), vectorSizeCompare);

  std::vector<std::pair<float, int> > profileResults;

  std::vector<Step> steps = path.getSteps();
  for (size_t i=0; i<allPaths.size(); ++i) {
    std::vector<std::pair<int, int> >& path = allPaths[i];
    steps.clear();
    for (size_t j=0; j<path.size(); ++j) {
      int v1 = path[j].first;
      int v2 = path[j].second;
      std::string n1 = g.getName(v1);
      std::string n2 = g.getName(v2);
      double weight = g.getWeight(v1, v2);
      AddStep(steps, v1, v2, n1, n2, weight, solutionIndex, true);
    }
    ComputeSolutionFast(route, steps);
    profileResults.push_back(std::make_pair(totalTime, i));
  }

  std::sort(profileResults.begin(), profileResults.end(), profileTimesCompare);

  if (debug) {
    std::cout << "ALL PATHS AND RESPECTIVE TIMINGS: " << std::endl;
    for (size_t i=0; i<profileResults.size(); ++i) {
      std::vector<std::pair<int, int> >& path = allPaths[profileResults[i].second];
      std::vector<std::pair<int, int> >::const_iterator it = path.begin();
      std::cout << map.getVertexName(it->first) << "->" << map.getVertexName(it->second) << " ";
      ++it;
      int steps = 1;
      for ( ; it != path.end(); ++it) {
        std::cout << map.getVertexName(it->second) << " ";
        steps++;
      }
      std::cout << "\tsteps: " << steps << " time: " << profileResults[i].first << std::endl;
    }
  }
  solutionNodeToStepMap.clear();
  solutionNodes.clear();
  solutionIndex = 0;
  steps.clear();
  std::vector<std::pair<int, int> >& path = allPaths[profileResults[0].second];
  for (size_t j=0; j<path.size(); ++j) {
    int v1 = path[j].first;
    int v2 = path[j].second;
    std::string n1 = g.getName(v1);
    std::string n2 = g.getName(v2);
    double weight = g.getWeight(v1, v2);
    AddStep(steps, v1, v2, n1, n2, weight, solutionIndex, true);
  }
  ComputeSolution(route, steps);
  OutputSolution(route);
  OutputGraph("exhaustive");
  OutputTimeElapsed();
}

//==============================================================================
//==============================================================================
template <class ParentDecorator>
struct print_parent {
  print_parent(const ParentDecorator& p_) : p(p_) { }
  template <class VertexDescriptor>
  void operator()(const VertexDescriptor& v) const {
    std::cout << "parent[" << v << "] = " <<  p[v]  << std::endl;
  }
  ParentDecorator p;
};


template <class NewGraph, class Tag>
struct graph_copier
  : public base_visitor<graph_copier<NewGraph, Tag> >
{
  typedef Tag event_filter;

  graph_copier(NewGraph& graph) : new_g(graph) { }

  template <class Edge, class t_graph>
  void operator()(Edge e, t_graph& g) {
    add_edge(source(e, g), target(e, g), new_g);
  }
private:
  NewGraph& new_g;
};

template <class NewGraph, class Tag>
inline graph_copier<NewGraph, Tag>
copy_graph(NewGraph& g, Tag) {
  return graph_copier<NewGraph, Tag>(g);
}

//==============================================================================
// BFS
//==============================================================================
//TODO: NOT YET WORKING...
void Planner::SolveBFS(Route& route) {
  start = bpt::microsec_clock::local_time();
  if (debug)
    std::cout << std::endl << "Calculating route using 'BFS' algorithm for: "
        << route << std::endl;

//  //typedef adjacency_list < vecS, vecS, undirectedS > t_graph;
  typedef float Weight;
  typedef property<edge_weight_t, Weight> WeightProperty;
  typedef property<vertex_name_t, std::string> NameProperty;

  typedef property<vertex_color_t, default_color_type, property<vertex_degree_t, int,
      property<vertex_in_degree_t, int, property<vertex_out_degree_t, int> > > > Whatever;

  // undirectedS directedS bidirectionalS vecS listS
  typedef adjacency_list<listS, vecS, directedS,
      NameProperty, WeightProperty, Whatever > t_graph;

  typedef graph_traits<t_graph>::vertex_descriptor VertexDescriptor;

  typedef property_map<t_graph, vertex_index_t>::type IndexMap;
  typedef property_map<t_graph, vertex_name_t>::type NameMap;

  typedef iterator_property_map<VertexDescriptor*, IndexMap, VertexDescriptor, VertexDescriptor&> PredecessorMap;
  typedef iterator_property_map<Weight*, IndexMap, Weight, Weight&> DistanceMap;

  const std::string routeNameDeparture = route.getCircleDeparture();
  const std::string routeNameArrival = route.getCircleArrival();

  const std::string srcVertexName = routeNameDeparture
      + lexical_cast<std::string>(route.getAngleDeparture());
  const std::string dstVertexName = routeNameArrival
      + lexical_cast<std::string>(route.getAngleArrival());

  // Create a graph
  t_graph g;
  VertexDescriptor vert;

  vertexDeptarture = Vertex(routeNameDeparture, srcVertexName, route.getAngleDeparture());
  vertexArrival = Vertex(routeNameArrival, dstVertexName, route.getAngleArrival());
  int solutionIndex = 0;

  FillVertices();
  for (std::map<std::string, Vertex>::iterator it=vertices.begin(); it!=vertices.end(); ++it) {
    vert = add_vertex(it->first, g);
    it->second.setVertex(vert);
    map.addItem(it->second.getRoadName(), it->first, vert, it->second.getAngle());
  }

  FillEdges();
  for (std::map<std::string, Edge>::iterator it=edges.begin(); it!=edges.end(); ++it) {
    add_edge(it->second.getV1(), it->second.getV2(), it->second.getWeight(), g);
    add_edge(it->second.getV2(), it->second.getV1(), it->second.getWeight(), g);
  }

  int graphSize = vertices.size();

  std::vector<Road> rds = roads->get();
  if (debug)
    std::cout << std::endl
        << "Calculating route using 'BFS' algorithm for: " << std::endl
        << route << std::endl << std::endl;

  // Create things for Dijkstra
  std::vector<VertexDescriptor> predecessors(num_vertices(g)); // To store parents
  std::vector<Weight> distances(num_vertices(g)); // To store distances

  IndexMap indexMap = get(vertex_index, g);
  PredecessorMap predecessorMap(&predecessors[0], indexMap);
  DistanceMap distanceMap(&distances[0], indexMap);

  const VertexDescriptor srcVertex = map.getNameVertex(srcVertexName);
  const VertexDescriptor dstVertex = map.getNameVertex(dstVertexName);

  t_graph G_copy(graphSize);
  std::vector<VertexDescriptor> p(boost::num_vertices(g));
  boost::graph_traits<t_graph>::vertices_size_type d[graphSize];
  std::fill_n(d, graphSize, 0);

  VertexDescriptor s = srcVertex;
  p[s] = s;

  breadth_first_search
      (g, srcVertex,
       visitor(make_bfs_visitor
       (std::make_pair(record_distances(d, on_tree_edge()),
                       std::make_pair
                       (record_predecessors(&p[0],
                                                   on_tree_edge()),
                        copy_graph(G_copy, on_examine_edge())))) ));

  // Extract a shortest path
  typedef std::vector<t_graph::edge_descriptor> PathType;
  PathType track;
  graph_traits<t_graph>::vertex_iterator i, iend;
  VertexDescriptor v = dstVertex; // We want to start at the destination and work our way back to the source
  for (VertexDescriptor u = p[v]; u != v; v = u, u = p[v]) {
    std::pair<t_graph::edge_descriptor, bool> edgePair = edge(u, v, g);
    t_graph::edge_descriptor edge = edgePair.first;
    track.push_back(edge);
  }
  NameMap nameMap = get(vertex_name, g);

  // Write shortest track
  std::vector<Step> steps = path.getSteps();
  if (routeNameDeparture != routeNameArrival) {
    if (debug)
      std::cout << "Shortest path from vS to vD:" << std::endl;
    for (PathType::reverse_iterator pathIterator = track.rbegin();
        pathIterator != track.rend(); ++pathIterator) {

      VertexDescriptor v1 = source(*pathIterator, g);
      VertexDescriptor v2 = target(*pathIterator, g);
      std::string n1 = nameMap[v1];
      std::string n2 = nameMap[v2];
      int weight = get(edge_weight, g, *pathIterator);

      AddStep(steps, v1, v2, n1, n2, weight, solutionIndex, false);
    }
    if (debug)
      std::cout << std::endl;
  } else {
    AddStepSourceIsDestination(route, steps, solutionIndex);
  }

  ComputeSolution(route, steps);
  OutputSolution(route);
  OutputGraph("bfs");
  OutputTimeElapsed();
}
//==============================================================================

//==============================================================================
// DIJKSTRA
//==============================================================================
void Planner::SolveDijkstra(Route& route) {
  start = bpt::microsec_clock::local_time();
  typedef float Weight;
  typedef property<edge_weight_t, Weight> WeightProperty;
  typedef property<vertex_name_t, std::string> NameProperty;

  // undirectedS directedS bidirectionalS vecS listS
  typedef adjacency_list<listS, vecS, directedS,
      NameProperty, WeightProperty> t_graph;

  typedef graph_traits<t_graph>::vertex_descriptor VertexDescriptor;

  typedef property_map<t_graph, vertex_index_t>::type IndexMap;
  typedef property_map<t_graph, vertex_name_t>::type NameMap;

  typedef iterator_property_map<VertexDescriptor*, IndexMap, VertexDescriptor, VertexDescriptor&> PredecessorMap;
  typedef iterator_property_map<Weight*, IndexMap, Weight, Weight&> DistanceMap;

  const std::string routeNameDeparture = route.getCircleDeparture();
  const std::string routeNameArrival = route.getCircleArrival();

  const std::string srcVertexName = routeNameDeparture
      + lexical_cast<std::string>(route.getAngleDeparture());
  const std::string dstVertexName = routeNameArrival
      + lexical_cast<std::string>(route.getAngleArrival());

  // Create a graph
  t_graph g;
  VertexDescriptor vert;

  vertexDeptarture = Vertex(routeNameDeparture, srcVertexName, route.getAngleDeparture());
  vertexArrival = Vertex(routeNameArrival, dstVertexName, route.getAngleArrival());
  int solutionIndex = 0;

  FillVertices();
  for (std::map<std::string, Vertex>::iterator it=vertices.begin(); it!=vertices.end(); ++it) {
    vert = add_vertex(it->first, g);
    it->second.setVertex(vert);
    map.addItem(it->second.getRoadName(), it->first, vert, it->second.getAngle());
  }

  FillEdges();
  for (std::map<std::string, Edge>::iterator it=edges.begin(); it!=edges.end(); ++it) {
    add_edge(it->second.getV1(), it->second.getV2(), it->second.getWeight(), g);
    add_edge(it->second.getV2(), it->second.getV1(), it->second.getWeight(), g);
  }

  std::vector<Road> rds = roads->get();
  if (debug)
    std::cout << std::endl
        << "Calculating route using 'Dijkstra' algorithm for: " << std::endl
        << route << std::endl << std::endl;

  // Create things for Dijkstra
  std::vector<VertexDescriptor> predecessors(num_vertices(g)); // To store parents
  std::vector<Weight> distances(num_vertices(g)); // To store distances

  IndexMap indexMap = get(vertex_index, g);
  PredecessorMap predecessorMap(&predecessors[0], indexMap);
  DistanceMap distanceMap(&distances[0], indexMap);

  const VertexDescriptor srcVertex = map.getNameVertex(srcVertexName);
  const VertexDescriptor dstVertex = map.getNameVertex(dstVertexName);

  // Compute shortest paths from v0 to all vertices, and store the output in predecessors and distances
  // dijkstra_shortest_paths(g, v0, predecessor_map(predecessorMap).distance_map(distanceMap));
  // This is exactly the same as the above line - it is the idea of "named parameters" - you can pass the
  // prdecessor map and the distance map in any order.
  dijkstra_shortest_paths(g, srcVertex,
      distance_map(distanceMap).predecessor_map(predecessorMap));

//  dijkstra_shortest_paths_no_color_map(g, srcVertex,
//      predecessor_map(make_iterator_property_map(predecessors.begin(), get(vertex_index, g))).
//      distance_map(make_iterator_property_map(distances.begin(), get(vertex_index, g))));

  // Extract a shortest path
  typedef std::vector<t_graph::edge_descriptor> PathType;
  PathType track;
  graph_traits<t_graph>::vertex_iterator i, iend;
  VertexDescriptor v = dstVertex; // We want to start at the destination and work our way back to the source
  for (VertexDescriptor u = predecessorMap[v]; u != v; v = u, u = predecessorMap[v]) {
    std::pair<t_graph::edge_descriptor, bool> edgePair = edge(u, v, g);
    t_graph::edge_descriptor edge = edgePair.first;
    track.push_back(edge);
  }
  NameMap nameMap = get(vertex_name, g);

  // Write shortest track
  std::vector<Step> steps = path.getSteps();
  if (routeNameDeparture != routeNameArrival) {
    if (debug)
      std::cout << "Shortest path from vS to vD:" << std::endl;
    for (PathType::reverse_iterator pathIterator = track.rbegin();
        pathIterator != track.rend(); ++pathIterator) {

      VertexDescriptor v1 = source(*pathIterator, g);
      VertexDescriptor v2 = target(*pathIterator, g);
      std::string n1 = nameMap[v1];
      std::string n2 = nameMap[v2];
      int weight = get(edge_weight, g, *pathIterator);

      AddStep(steps, v1, v2, n1, n2, weight, solutionIndex, false);
    }
    if (debug)
      std::cout << std::endl;
  } else {
    AddStepSourceIsDestination(route, steps, solutionIndex);
  }

  ComputeSolution(route, steps);
  OutputSolution(route);
  OutputGraph("dijkstra");
  OutputTimeElapsed();
}

//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================
void Planner::OutputTimeElapsed() {
  stop = bpt::microsec_clock::local_time();
  bpt::time_duration dur = stop - start;
  long milliseconds = dur.total_milliseconds();
  std::cout << "time elapsed: " << std::setprecision(2) << milliseconds << "ms" << std::endl;
}

//==============================================================================
void Planner::AddStep(std::vector<Step>& steps, int v1, int v2,
    std::string n1, std::string n2, double weight, int& solutionIndex, bool supressOutput=false) {
  Road * srcRoad = roads->getByName(map.getVertexRoadName(v1));
  Road * dstRoad = roads->getByName(map.getVertexRoadName(v2));
  const bool isIntersection = srcRoad->getName() != dstRoad->getName();
  if (isIntersection) {
    //intersections have close to zero cost, and no weight
    weight = 0.0;
  }
  if (debug && !supressOutput)
    std::cout << n1 << " -> " << n2 << " @ " << weight << std::endl;
  int srcPoint = map.getVertexAngle(v1);
  int dstPoint = map.getVertexAngle(v2);
  steps.push_back(
      Step(srcRoad, dstRoad, srcPoint, dstPoint, isIntersection, weight,
          debug));
  std::string n1n2 = n1 + n2;
  solutionNodeToStepMap[n1n2] = solutionIndex++;
  solutionNodes.insert(n1);
  solutionNodes.insert(n2);
}

//==============================================================================
void Planner::AddStepSourceIsDestination(Route& route, std::vector<Step>& steps,
    int& solutionIndex) {
  //destination is same as source
  Road * srcRoad = roads->getByName(route.getCircleDeparture());
  int angleA = route.getAngleDeparture();
  int angleB = route.getAngleArrival();
  double weight = calculate_weight(srcRoad->getSegmentSize(), angleA, angleB);
  steps.push_back(
      Step(srcRoad, srcRoad, route.getAngleDeparture(),
          route.getAngleArrival(), false, weight,
          debug));
  std::string n1n2 = srcRoad->getName() + srcRoad->getName();
  solutionNodeToStepMap[n1n2] = solutionIndex++;
  solutionNodes.insert(route.getCircleDeparture());
  solutionNodes.insert(route.getCircleArrival());
}

//==============================================================================
void Planner::ComputeSolution(Route& route, std::vector<Step>& steps) {
  int direction = route.getDirection();
  if (debug)
    std::cout << "COMPUTING" << std::endl;
  //compute path taking into account direction shifts, etc...
  bool started = false;
  totalTime = 0.0;
  commands.clear();
  for (size_t i = 0; i < steps.size(); ++i) {
    direction = steps[i].CalculateCommand(direction, started, i);
    totalTime += steps[i].getTime();
    commands += steps[i].getCommand();
  }
}

//==============================================================================
void Planner::ComputeSolutionFast(Route& route, std::vector<Step>& steps) {
  int direction = route.getDirection();
  bool started = false;
  totalTime = 0.0;
  for (size_t i = 0; i < steps.size(); ++i) {
    direction = steps[i].CalculateCommandFast(direction, started, i);
    totalTime += steps[i].getTime();
  }
}

//==============================================================================
void Planner::OutputSolution(Route& route) {
  if (debug) {
    std::vector<Step> steps = path.getSteps();
    std::cout << std::endl << "STEPS: " << std::endl;
    for (size_t i = 0; i < steps.size(); ++i) {
      std::cout << steps[i] << std::endl;
    }
    std::cout << std::endl << "ROUTE: ";
    std::cout << "Start: ";
    if (route.getDirection() == 0) {
      std::cout << "CW ";
    } else {
      std::cout << "CCW ";
    }
    std::cout << route.getCircleDeparture() << route.getAngleDeparture();
    std::cout << " -> " << route.getCircleArrival() << route.getAngleArrival()
        << std::endl;
    std::cout << std::endl << "RESULT:" << std::endl;

  }
  std::cout << std::fixed << std::setprecision(1) << totalTime << "\n"
      << commands << std::endl;
  if (debug)
    std::cout << std::endl;
}

void Planner::OutputGraph(std::string fileName) {

  std::set<std::string> sweights;
  std::set<std::string> transfers;
  std::map<std::string, std::string> nodeToCircleNameMap;

  std::string outFileDot = "./" + fileName + ".dot";

  std::ofstream dot_file(outFileDot.c_str());
  dot_file << "digraph D {\n" << "  rankdir=LR\n" << "  size=\"14,14\"\n"
      << "  ratio=\"fill\"\n" << "  edge[style=\"solid\" arrowhead=onormal penwidth=1.0 ]\n"
      << "  node[shape=\"circle\" color=\"gray55\"  penwidth=2.0 fontcolor=grey20 fontsize=16"
      << " fixedsize=true width=1.0  height=1.0 ]\n";

  //output rank, to be able to group circle nodes together
  for (std::set<std::string>::iterator it = cset.begin(); it != cset.end();
      ++it) {
    std::pair<std::multimap<std::string, std::string>::iterator,
        std::multimap<std::string, std::string>::iterator> ret;
    ret = circleToNodeMap.equal_range(*it);
    dot_file << "\t{rank=same; ";
    for (std::multimap<std::string, std::string>::iterator it = ret.first;
        it != ret.second; ++it) {
      dot_file << it->second << " ";
      nodeToCircleNameMap[it->second] = it->first;
    }
    dot_file << "}\n";
  }

  //color solution nodes blue
  for (std::set<std::string>::iterator cit = solutionNodes.begin();
      cit != solutionNodes.end(); ++cit) {
    dot_file << "\t" << *cit << " [label=\"" << *cit
        << "\", color=\"blue\", penwidth=3.0, shape=circle" << "]\n";
  }

  //special cases for start and destination nodes
  dot_file << "\n\t" << vertexDeptarture.getRoadName() << vertexDeptarture.getAngle() << "[label=\""
      << vertexDeptarture.getRoadName() << vertexDeptarture.getAngle()
      << "\", color=\"blue\", penwidth=2.0, shape=doublecircle fontcolor=black fontsize=24 "
      << "]\n";
  dot_file << "\t" << vertexArrival.getRoadName() << vertexArrival.getAngle() << " [label=\""
      << vertexArrival.getRoadName() << vertexArrival.getAngle()
      << "\", color=\"blue\", penwidth=2.0, shape=doubleoctagon fontcolor=black fontsize=24"
      << "]\n\n";

  //the rest
  for (std::map<std::string, Edge>::iterator it=edges.begin(); it!=edges.end(); ++it) {
    Edge& edge = it->second;

    std::string sNd = edge.getName1();
    std::string dNd = edge.getName2();
    dot_file << sNd << " -> " << dNd << "\t[label=\"";

    std::map<std::string, int>::iterator it2;
    it2 = solutionNodeToStepMap.find(sNd + dNd);
    int iWeight = (int) edge.getWeight();
    std::string sWeight = nodeToCircleNameMap[sNd]
        + lexical_cast<std::string>(iWeight);
    std::string sTransfer1 = sNd + dNd;
    std::string sTransfer2 = dNd + sNd;
    if (it2 != solutionNodeToStepMap.end()) {
      //edge is solution
      if (iWeight == 0) {
        const bool found = transfers.find(sTransfer1) != transfers.end();
        if (!found) {
          dot_file << "TRANSFER" << "\""
              << ", fontcolor=gray fontsize=12 style=dashed penwidth=1.0 color=\"blue\"";
          transfers.insert(sTransfer1);
          transfers.insert(sTransfer2);
        } else {
          dot_file << "" << "\""
              << ", fontcolor=gray fontsize=12 style=dashed penwidth=1.0 color=\"blue\"";
        }
      } else {
        const bool found = sweights.find(sWeight) != sweights.end();
        if (!found) {
          dot_file << iWeight << "\"" << ", penwidth=1.5 color=\"blue\"";
          sweights.insert(sWeight);
        } else {
          dot_file << "\"" << ", penwidth=2.0 color=\"blue\"";
        }
      }
    } else {
      if (iWeight == 0) {
        const bool found = transfers.find(sTransfer1) != transfers.end();
        if (!found) {
          dot_file << "TRANSFER" << "\""
              << ", fontcolor=gray fontsize=12 style=dashed penwidth=0.5 color=\"gray44\"";
          transfers.insert(sTransfer1);
          transfers.insert(sTransfer2);
        } else {
          dot_file << "" << "\""
              << ", fontcolor=gray fontsize=12 style=dashed penwidth=0.5 color=\"gray44\"";
        }
      } else {
        const bool found = sweights.find(sWeight) != sweights.end();
        if (!found) {
          dot_file << iWeight << "\"" << ", penwidth=0.5 color=\"gray44\"";
          sweights.insert(sWeight);
        } else {
          dot_file << "\"" << ", penwidth=0.5 color=\"gray44\"";
        }
      }
    }
    dot_file << "]" << std::endl;
  }
  dot_file << "}";
}

//==============================================================================
//==============================================================================
void Planner::FillVertices() {
  std::vector<Road> rds = roads->get();
  if (debug)
    std::cout << "ADDING VERTICES" << std::endl;

  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Section> sections = rds[i].getSections();
    for (size_t j = 0; j < sections.size(); ++j) {
      std::vector<Section> sections = rds[i].getSections();
      cset.insert(sections[j].getRoadA());
      cset.insert(sections[j].getRoadB());
    }
  }
  for (std::set<std::string>::iterator cit = cset.begin(); cit != cset.end();
      ++cit) {
    circleToNodeMap.insert(std::make_pair(*cit, ""));
  }

  int idx = 0;
  std::set<std::string> vset;

  vertices.insert(std::pair<std::string, Vertex>(vertexDeptarture.getName(), vertexDeptarture));
  vset.insert(vertexDeptarture.getName());
  idx++;
  if (debug)
    std::cout << "\t" << vertexDeptarture.getName() << std::endl;

  vertices.insert(std::pair<std::string, Vertex>(vertexArrival.getName(), vertexArrival));
  vset.insert(vertexArrival.getName());
  idx++;
  if (debug)
    std::cout << "\t" << vertexArrival.getName() << std::endl;

  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Section> sections = rds[i].getSections();
    for (size_t j = 0; j < sections.size(); ++j) {
      std::string n1 = sections[j].getRoadA()
          + lexical_cast<std::string>(sections[j].getPointA());
      const bool found1 = vset.find(n1) != vset.end();
      if (!found1) {
        if (debug)
          std::cout << "\t" << n1 << std::endl;
        vset.insert(n1);
        vertices.insert(std::pair<std::string, Vertex>(n1, Vertex(sections[j].getRoadA(), n1, sections[j].getPointA())));
        circleToNodeMap.insert(std::make_pair(sections[j].getRoadA(), n1));
        idx++;
      }
      std::string n2 = sections[j].getRoadB()
          + lexical_cast<std::string>(sections[j].getPointB());
      const bool found2 = vset.find(n2) != vset.end();
      if (!found2) {
        if (debug)
          std::cout << "\t" << n2 << std::endl;
        vset.insert(n2);
        vertices.insert(std::pair<std::string, Vertex>(n2, Vertex(sections[j].getRoadB(), n2, sections[j].getPointB())));
        circleToNodeMap.insert(std::make_pair(sections[j].getRoadB(), n2));
        idx++;
      }
    }
  }
  if (debug)
    std::cout << "Found: " << idx << " vertices" << std::endl << std::endl;
}

//==============================================================================
//==============================================================================
void Planner::FillEdges() {
  std::vector<Road> rds = roads->get();

  int idx = 0;
  //std::set<std::string> vset;

  std::string n1;
  std::string n2;
  idx = 0;
  std::set<std::string> selfEdges;
  std::set<std::string> dispEdges;
  if (debug)
    std::cout << "ADDING EDGES" << std::endl;
  if (debug)
    std::cout << "  Displacement Edges" << std::endl;
  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Section> sections = rds[i].getSections();
    for (size_t j = 0; j < sections.size(); ++j) {
      std::string n1 = sections[j].getRoadA()
          + lexical_cast<std::string>(sections[j].getPointA());
      selfEdges.insert(n1);
    }
    //create an edge linking all
    std::set<std::string>::iterator it;
    std::set<std::string>::iterator it2;
    std::set<std::string>::iterator itLast = selfEdges.end();
    --itLast;
    for (it = selfEdges.begin(); it != selfEdges.end(); ++it) {
      n1 = *it;
      for (it2 = it, ++it2; it2 != itLast, it2 != selfEdges.end(); ++it2) {
        n2 = *it2;

        int v1 = map.getNameVertex(*it);
        int v2 = map.getNameVertex(*it2);

        int angleA = map.getVertexAngle(v1);
        int angleB = map.getVertexAngle(v2);
        if (angleB > 180 && angleA < 180) {
          angleA += 180;
        }
        std::string n1n2 = n1 + n2;
        std::string n2n1 = n2 + n1;
        //calculate actual weight, this is based on the distance from one angle to the other
        double w = calculate_weight(rds[i].getSegmentSize(), angleA, angleB);
        edges.insert(std::pair<std::string, Edge>(n1n2, Edge(n1, n2, v1, v2, w)));
        edges.insert(std::pair<std::string, Edge>(n2n1, Edge(n2, n1, v2, v1, w)));
        dispEdges.insert(n1n2);
        if (debug)
          std::cout << "\t" << *it << "<->" << *it2 << " weight " << w << std::endl;
        idx++;
      }
    }
    selfEdges.clear();
  }
  int dsedges = idx;

  if (debug)
    std::cout << "  intersection Edges" << std::endl;
  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Section> sections = rds[i].getSections();
    for (size_t j = 0; j < sections.size(); ++j) {
      std::string n1 = sections[j].getRoadA()
          + lexical_cast<std::string>(sections[j].getPointA());
      std::string n2 = sections[j].getRoadB()
          + lexical_cast<std::string>(sections[j].getPointB());
      int v1 = map.getNameVertex(n1);
      int v2 = map.getNameVertex(n2);
      if (debug)
        std::cout << "\t" << n1 << "<->" << n2 << std::endl;
      edges.insert(std::pair<std::string, Edge>(n1+n2, Edge(n1, n2, v1, v2, 0.0)));
      dispEdges.insert(n1+n2);
      idx++;
      map.addIntersection(n1);
      map.addIntersection(n2);
    }
  }
  int ixedges = idx - dsedges;
  if (debug)
    std::cout << "  End Edges" << std::endl;
  selfEdges.clear();
  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Section> sections = rds[i].getSections();
    for (size_t j = 0; j < sections.size(); ++j) {
      std::string n1 = sections[j].getRoadA()
          + lexical_cast<std::string>(sections[j].getPointA());
      std::string n2 = "";
      int pointB = 0;
      if (vertexDeptarture.getRoadName() == sections[j].getRoadA() && !map.hasIntersection(vertexDeptarture.getName())) {
        n2 = vertexDeptarture.getName();
        pointB = vertexDeptarture.getAngle();
      } else if (vertexArrival.getRoadName() == sections[j].getRoadA() && !map.hasIntersection(vertexArrival.getName())) {
        n2 = vertexArrival.getName();
        pointB = vertexArrival.getAngle();
      }
      if (!n2.empty()) {
        const bool found1 = dispEdges.find(n1) != dispEdges.end();
        if (!found1) {
          int v1 = map.getNameVertex(n1);
          int v2 = map.getNameVertex(n2);
          int pointA = sections[j].getPointA();
          double w = calculate_weight(rds[i].getSegmentSize(), pointA, pointB);
          edges.insert(std::pair<std::string, Edge>(n1+n2, Edge(n1, n2, v1, v2, w)));
          edges.insert(std::pair<std::string, Edge>(n2+n1, Edge(n2, n1, v2, v1, w)));
          dispEdges.insert(n1);
          if (debug)
            std::cout << "\t" << n1 << "<->" << n2  << " weight " << w << std::endl;
          idx++;
        }
      }
    }
  }
  int eedges = idx - ixedges - dsedges;
  if (debug) {
    std::cout << "Found: " << dsedges << " displacement sections" << std::endl;
    std::cout << "Found: " << ixedges << " intersection sections" << std::endl;
    std::cout << "Found: " << eedges << " end sections" << std::endl;
    std::cout << "Found: " << idx << " sections" << std::endl;
  }
}

