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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
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
#include "Step.h"
#include "Edge.h"

using namespace boost;

Planner::~Planner() {
}

void Planner::Solve(Route& route, int algorithm) {
  switch (algorithm) {
  case CIRCLE_ONLY:
    SolveCircleOnly(route);
    break;
  case BFS:
    SolveBFS(route);
    break;
  case DIJKSTRA:
    SolveDijkstra(route);
    break;
  }
}

void Planner::SolveCircleOnly(Route& route) {
  std::cout << std::endl
      << "Calculating route using 'Circle Only' algorithm for: " << route
      << std::endl;
}

//==============================================================================
// DIJKSTRA
//==============================================================================
void Planner::SolveDijkstra(Route& route) {

  typedef float Weight;
  typedef boost::property<boost::edge_weight_t, Weight> WeightProperty;
  typedef boost::property<boost::vertex_name_t, std::string> NameProperty;

  // undirectedS directedS bidirectionalS
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
      NameProperty, WeightProperty> Graph;

  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

  typedef boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
  typedef boost::property_map<Graph, boost::vertex_name_t>::type NameMap;

  typedef boost::iterator_property_map<Vertex*, IndexMap, Vertex, Vertex&> PredecessorMap;
  typedef boost::iterator_property_map<Weight*, IndexMap, Weight, Weight&> DistanceMap;

  const std::string routeNameDeparture = route.getCircleDeparture();
  const std::string routeNameArrival = route.getCircleArrival();
  const int routeDepartureAngle = route.getAngleDeparture();
  const int routeArrivalAngle = route.getAngleArrival();

  // Create a graph
  Graph g;

  std::vector<Road> rds = roads->get();
  if (debug)
    std::cout << "ADDING VERTICES" << std::endl;

  std::set<std::string> cset;
  std::multimap<std::string, std::string> circleToNodeMap;

  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Edge> edges = rds[i].getEdges();
    for (size_t j = 0; j < edges.size(); ++j) {
      std::vector<Edge> edges = rds[i].getEdges();
      cset.insert(edges[j].getRoadA());
      cset.insert(edges[j].getRoadB());
    }
  }
  for (std::set<std::string>::iterator cit = cset.begin(); cit != cset.end();
      ++cit) {
    circleToNodeMap.insert(std::make_pair(*cit, ""));
  }
  int idx = 0;
  std::set<std::string> vset;
  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Edge> edges = rds[i].getEdges();
    for (size_t j = 0; j < edges.size(); ++j) {
      std::string n1 = edges[j].getRoadA()
          + lexical_cast<std::string>(edges[j].getPointA());
      const bool found1 = vset.find(n1) != vset.end();
      if (!found1) {
        if (debug)
          std::cout << "\t" << n1 << std::endl;
        vset.insert(n1);
        Vertex v1 = boost::add_vertex(n1, g);
        map.addNameVertex(n1, v1);
        map.addIndexName(idx, n1);
        map.addVertexAngle(v1, edges[j].getPointA());
        map.addVertexRoadName(v1, edges[j].getRoadA());
        circleToNodeMap.insert(std::make_pair(edges[j].getRoadA(), n1));
        idx++;
      }
      std::string n2 = edges[j].getRoadB()
          + lexical_cast<std::string>(edges[j].getPointB());
      const bool found2 = vset.find(n2) != vset.end();
      if (!found2) {
        if (debug)
          std::cout << "\t" << n2 << std::endl;
        vset.insert(n2);
        Vertex v2 = boost::add_vertex(n2, g);
        map.addNameVertex(n2, v2);
        map.addIndexName(idx, n2);
        map.addVertexAngle(v2, edges[j].getPointB());
        map.addVertexRoadName(v2, edges[j].getRoadB());
        circleToNodeMap.insert(std::make_pair(edges[j].getRoadB(), n2));
        idx++;
      }
      if (edges[j].getRoadA() == routeNameDeparture) {
        route.DepartureClosestMatchingAngle(edges[j].getPointA());
      } else if (edges[j].getRoadB() == routeNameDeparture) {
        route.DepartureClosestMatchingAngle(edges[j].getPointB());
      }
      if (edges[j].getRoadA() == routeNameArrival) {
        route.ArrivalClosestMatchingAngle(edges[j].getPointA());
      } else if (edges[j].getRoadB() == routeNameArrival) {
        route.ArrivalClosestMatchingAngle(edges[j].getPointB());
      }
    }
  }
  if (debug)
    std::cout << "Found: " << idx << " vertices" << std::endl << std::endl;

  std::string n1;
  std::string n2;
  idx = 0;
  std::set<std::string> selfEdges;
  if (debug) {
    std::cout << "ADDING EDGES" << std::endl;
    std::cout << "\tDisplacement Edges" << std::endl;
  }
  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Edge> edges = rds[i].getEdges();
    for (size_t j = 0; j < edges.size(); ++j) {
      std::string n1 = edges[j].getRoadA()
          + lexical_cast<std::string>(edges[j].getPointA());
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

        Vertex v1 = map.getNameVertex(*it);
        Vertex v2 = map.getNameVertex(*it2);

        double weight = 0.0;
        int angleA = map.getVertexAngle(v1);
        int angleB = map.getVertexAngle(v2);
        if (angleB > 180 && angleA < 180) {
          angleA += 180;
        }
        //calculate actual weight, this is based on the distance from one angle to the other
        double segmentSize = rds[i].getSegmentSize();
        int angleDistance = angular_distance(angleA, angleB);
        weight = angleDistance * segmentSize;
        if (weight < 0.0)
          weight = 0.0;
        Weight w = weight;
        boost::add_edge(v1, v2, w, g);
        boost::add_edge(v2, v1, w, g);
        if (debug)
          std::cout << "\t" << *it << "<->" << *it2;
        if (debug)
          std::cout << "\t\tdistance " << angleDistance << " segmentSize "
              << segmentSize << " weight " << weight << std::endl;
        idx++;
      }
    }
    selfEdges.clear();
  }

  if (debug)
    std::cout << "\tintersection Edges" << std::endl;
  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Edge> edges = rds[i].getEdges();
    for (size_t j = 0; j < edges.size(); ++j) {
      std::string n1 = edges[j].getRoadA()
          + lexical_cast<std::string>(edges[j].getPointA());
      std::string n2 = edges[j].getRoadB()
          + lexical_cast<std::string>(edges[j].getPointB());
      std::string n1n2 = n1 + n2;
      if (!map.hasIntersection(n1n2)) {
        Vertex v1 = map.getNameVertex(n1);
        Vertex v2 = map.getNameVertex(n2);
        Weight w = 0.0;
        if (debug)
          std::cout << "\t" << n1 << "<->" << n2 << std::endl;

        boost::add_edge(v2, v1, w, g);
        idx++;
      }
      map.addIntersection(n1n2);
    }
  }
  if (debug)
    std::cout << "Found: " << idx << " edges" << std::endl;

  if (debug)
    std::cout << std::endl
        << "Calculating route using 'Dijkstra2' algorithm for: " << std::endl
        << route << std::endl << std::endl;

  // Create things for Dijkstra
  std::vector<Vertex> predecessors(boost::num_vertices(g)); // To store parents
  std::vector<Weight> distances(boost::num_vertices(g)); // To store distances

  IndexMap indexMap = boost::get(boost::vertex_index, g);
  PredecessorMap predecessorMap(&predecessors[0], indexMap);
  DistanceMap distanceMap(&distances[0], indexMap);

  const std::string srcVertexName = routeNameDeparture
      + lexical_cast<std::string>(route.getAngleDepartureClosestMatch());
  const std::string dstVertexName = routeNameArrival
      + lexical_cast<std::string>(route.getAngleArrivalClosestMatch());

  const Vertex srcVertex = map.getNameVertex(srcVertexName);
  const Vertex dstVertex = map.getNameVertex(dstVertexName);
  const int srcVertexAngle = map.getVertexAngle(srcVertex);
  const int dstVertexAngle = map.getVertexAngle(dstVertex);
  const std::string roadNameDeparture = map.getVertexRoadName(srcVertex);
  const std::string roadNameArrival = map.getVertexRoadName(dstVertex);

  // Compute shortest paths from v0 to all vertices, and store the output in predecessors and distances
  // boost::dijkstra_shortest_paths(g, v0, boost::predecessor_map(predecessorMap).distance_map(distanceMap));
  // This is exactly the same as the above line - it is the idea of "named parameters" - you can pass the
  // prdecessor map and the distance map in any order.
  boost::dijkstra_shortest_paths(g, srcVertex,
      boost::distance_map(distanceMap).predecessor_map(predecessorMap));

  if (debug)
    boost::print_graph(g);

  // Output results
  if (debug)
    std::cout << std::endl << "distances and parents:" << std::endl;
  NameMap nameMap = boost::get(boost::vertex_name, g);

  if (debug) {
    BGL_FORALL_VERTICES(v, g, Graph){
    std::cout << "distance(" << nameMap[srcVertex] << ", " << nameMap[v] << ") = " << distanceMap[v] << ", ";
    std::cout << "predecessor(" << nameMap[v] << ") = " << nameMap[predecessorMap[v]] << std::endl;
  }
}

  if (debug)
    std::cout << std::endl;

  // Extract a shortest path
  graph_traits<Graph>::vertex_iterator i, iend;
  typedef std::vector<Graph::edge_descriptor> PathType;

  PathType track;

  Vertex v = dstVertex; // We want to start at the destination and work our way back to the source
  for (Vertex u = predecessorMap[v]; // Start by setting 'u' to the destintaion node's predecessor
  u != v; // Keep tracking the track until we get to the source
      v = u, u = predecessorMap[v]) // Set the current vertex to the current predecessor, and the predecessor to one level up
      {
    std::pair<Graph::edge_descriptor, bool> edgePair = boost::edge(u, v, g);
    Graph::edge_descriptor edge = edgePair.first;
    track.push_back(edge);
  }

  std::vector<Step> steps = path.get();

  Road * roadDeparture = roads->getByName(routeNameDeparture);
  Road * roadArrival = roads->getByName(routeNameArrival);

  std::map<std::string, int> solutionNodeToStepMap;
  //if the source angles were changed because there wasn't an exact match in the Graph
  //if first step it's an intersection, add a new step, otherwise
  // update that first step with correct angle location, to be done later, since
  // there are no steps created yet
  bool updateStepLater = false;
  int solutionIndex = 0;
  //we need to find out the source vertex (the first one found by dijkstra) and get:
  // road names, and angles, we need to determine if it's an intersection
  if (srcVertexAngle != routeDepartureAngle) {
    std::string srcE;
    std::string dstE;
    int srcP = 0;
    int dstP = 0;
    for (PathType::reverse_iterator pathIterator3 = track.rbegin();
        pathIterator3 != track.rend(); ++pathIterator3) {
      srcE = nameMap[boost::source(*pathIterator3, g)];
      dstE = nameMap[boost::target(*pathIterator3, g)];
      srcP = map.getVertexAngle(source(*pathIterator3, g));
      dstP = map.getVertexAngle(target(*pathIterator3, g));
      break;
    }
    bool isFirstEdgeIntersection = map.hasIntersection(srcE + dstE);
    //double check via angular distances
    int d = angular_distance(srcP, dstP);
    if (d != 0 && d != 180) {
      isFirstEdgeIntersection = false;
    }
    std::string vertexRoadName = map.getVertexRoadName(srcVertex);
    int angleDistance = angular_distance(srcVertexAngle, routeDepartureAngle);

    //if the road names don't match, we'll have to add a step to move to intersecion location
    //add new only when first step would be an intersection, otherwise update later
    if (isFirstEdgeIntersection) {
      double srcWeight = roadDeparture->getSegmentSize() * angleDistance;
      steps.push_back(
          Step(roadDeparture, roadDeparture, routeDepartureAngle,
              srcVertexAngle, false, srcWeight, debug));
    } else {
      updateStepLater = true;
    }
  }

  std::set<std::string> solutionNodes;
  if (routeNameDeparture != routeNameArrival) {
    // Write shortest track
    if (debug)
      std::cout << "Shortest path from vS to vD:" << std::endl;
    for (PathType::reverse_iterator pathIterator2 = track.rbegin();
        pathIterator2 != track.rend(); ++pathIterator2) {

      std::string srcEdge = nameMap[boost::source(*pathIterator2, g)];
      std::string dstEdge = nameMap[boost::target(*pathIterator2, g)];
      int weight = boost::get(boost::edge_weight, g, *pathIterator2);
      if (debug)
        std::cout << srcEdge << " -> " << dstEdge << " @ " << weight << std::endl;

      //intersection should have different source and destination roads
      //non intersections always should have the same source and destination roads
      Road * srcRoad = roads->getByName(
          map.getVertexRoadName(source(*pathIterator2, g)));
      Road * dstRoad = roads->getByName(
          map.getVertexRoadName(target(*pathIterator2, g)));
      std::string n1n2 = srcEdge + dstEdge;
      solutionNodes.insert(srcEdge);
      solutionNodes.insert(dstEdge);
      const bool isIntersection = map.hasIntersection(n1n2);
      if (isIntersection) {
        //intersections have close to zero cost, and no weight
        weight = 0.0;
        if (srcRoad == dstRoad) {
          throw std::out_of_range("Intersection cannot be in the same road!");
        }
      } else {
        if (srcRoad != dstRoad) {
          throw std::out_of_range(
              "Displacement edge cannot be in different roads!");
        }
      }
      int srcPoint = map.getVertexAngle(source(*pathIterator2, g));
      int dstPoint = map.getVertexAngle(target(*pathIterator2, g));
      steps.push_back(
          Step(srcRoad, dstRoad, srcPoint, dstPoint, isIntersection, weight,
              debug));
      solutionNodeToStepMap[n1n2] = solutionIndex++;
    }
    if (debug)
      std::cout << std::endl;
  }

  if (updateStepLater) {
    int distanceA = angular_distance(steps[0].getPointDestination(),
        routeDepartureAngle);
    if (distanceA) {
      steps[0].setPointSource(routeDepartureAngle);
      int curDestinationPoint = steps[0].getPointDestination();
      distanceA = angular_distance(routeDepartureAngle, curDestinationPoint);
      double weightDeparture = roadDeparture->getSegmentSize() * distanceA;
      steps[0].setWeight(weightDeparture);
    }
  }

  if (routeNameDeparture != routeNameArrival) {
    //if the destination angles were changed because there wasn't an exact match in the Graph
    //add the last step to go from that angle into the one in the graph, which is a transfer
    //location or intersection
    if (dstVertexAngle != routeArrivalAngle) {
      //if last step is an intersection we'll have to add a new step
      if (steps[steps.size() - 1].isIntersection()) {
        int angleDistance = angular_distance(routeArrivalAngle, dstVertexAngle);
        double dstWeight = roadArrival->getSegmentSize() * angleDistance;
        steps.push_back(
            Step(roadArrival, roadArrival, dstVertexAngle, routeArrivalAngle,
                false, dstWeight, debug));
      } else {
        int distanceB = angular_distance(routeArrivalAngle,
            route.getAngleArrivalClosestMatch());
        if (distanceB) {
          steps[steps.size() - 1].setPointDestination(routeArrivalAngle);
          int curSourcePoint = steps[steps.size() - 1].getPointSource();
          distanceB = angular_distance(routeArrivalAngle, curSourcePoint);
          double weightArrival = roadArrival->getSegmentSize() * distanceB;
          steps[steps.size() - 1].setWeight(weightArrival);
        }
      }
    }
  } else {
    int distance = angular_distance(routeDepartureAngle, routeArrivalAngle);
    double weight = roadDeparture->getSegmentSize() * distance;
    steps.push_back(
        Step(roadDeparture, roadDeparture, routeDepartureAngle,
            routeArrivalAngle, false, weight, debug));
  }

  int direction = route.getDirection();

  //compute path taking into account direction shifts, etc...
  bool started = false;
  double totalTime = 0.0;
  std::string commands;
  for (size_t i = 0; i < steps.size(); ++i) {
    direction = steps[i].CalculateCommand(direction, started, i);
    totalTime += steps[i].getTime();
    commands += steps[i].getCommand();
  }
  if (debug) {
    std::cout << std::endl << "STEPS: " << std::endl;
    for (size_t i = 0; i < steps.size(); ++i) {
      std::cout << steps[i] << std::endl;
    }
    std::cout << std::endl << "ROUTE: ";
    std::cout << "Start: ";
    if (route.getDirection() == 0) {
      std::cout << "+ ";
    } else {
      std::cout << "- ";
    }
    std::cout << route.getCircleDeparture() << route.getAngleDeparture();
    std::cout << " -> " << route.getCircleArrival() << route.getAngleArrival()
        << std::endl;
    std::cout << std::endl << "RESULT:" << std::endl;

  }

  if (debug)
    std::cout << std::fixed << std::setprecision(1) << totalTime << "\n\n"
        << commands << std::endl;
  else
    std::cout << std::fixed << std::setprecision(1) << totalTime << "\n"
        << commands << std::endl;

  std::set<std::string> sweights;
  std::set<std::string> transfers;
  std::map<std::string, std::string> nodeToCircleNameMap;
  property_map<Graph, edge_weight_t>::type weightmap = get(edge_weight, g);
  std::ofstream dot_file("./dijkstra.dot");
  dot_file << "digraph D {\n" << "  rankdir=LR\n" << "  size=\"14,14\"\n"
      << "  ratio=\"fill\"\n" << "  edge[style=\"solid\"]\n"
      << "  node[shape=\"circle\"  width=0.5 fontcolor=black fontsize=16 fixedsize=true width=1.0  height=1.0 ]\n";

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

  for (std::set<std::string>::iterator cit = solutionNodes.begin();
      cit != solutionNodes.end(); ++cit) {
    dot_file << "\t" << *cit << " [label=\"" << *cit
        << "\", color=\"blue\", penwidth=3.0, shape=circle" << "]\n";
  }
  dot_file << "\n\t" << roadNameDeparture << srcVertexAngle << "[label=\""
      << roadNameDeparture << srcVertexAngle
      << "\", color=\"blue\", penwidth=2.0, shape=doublecircle fontcolor=blue fontsize=22 "
      << "]\n";
  dot_file << "\t" << roadNameArrival << dstVertexAngle << " [label=\""
      << roadNameArrival << dstVertexAngle
      << "\", color=\"blue\", penwidth=2.0, shape=doubleoctagon fontcolor=blue fontsize=22"
      << "]\n\n";

  graph_traits<Graph>::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    graph_traits<Graph>::edge_descriptor e = *ei;
    graph_traits<Graph>::vertex_descriptor u = boost::source(e, g), v =
        boost::target(e, g);

    std::string sNd = map.getIndexName(u);
    std::string dNd = map.getIndexName(v);
    dot_file << sNd << " -> " << dNd << "\t[label=\"";

    std::map<std::string, int>::iterator it;
    it = solutionNodeToStepMap.find(sNd + dNd);
    int iWeight = (int) get(weightmap, e);
    std::string sWeight = nodeToCircleNameMap[sNd]
        + lexical_cast<std::string>(iWeight);
    std::string sTransfer1 = sNd + dNd;
    std::string sTransfer2 = dNd + sNd;
    if (it != solutionNodeToStepMap.end()) {
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
          dot_file << iWeight << "\"" << ", penwidth=3.0 color=\"blue\"";
          sweights.insert(sWeight);
        } else {
          dot_file << "\"" << ", penwidth=3.0 color=\"blue\"";
        }
      }
    } else {
      if (iWeight == 0) {
        const bool found = transfers.find(sTransfer1) != transfers.end();
        if (!found) {
          dot_file << "TRANSFER" << "\""
              << ", fontcolor=gray fontsize=12 style=dotted penwidth=1.5 color=\"black\"";
          transfers.insert(sTransfer1);
          transfers.insert(sTransfer2);
        } else {
          dot_file << "" << "\""
              << ", fontcolor=gray fontsize=12 style=dotted penwidth=1.5 color=\"black\"";
        }
      } else {
        const bool found = sweights.find(sWeight) != sweights.end();
        if (!found) {
          dot_file << iWeight << "\"" << ", penwidth=0.5 color=\"black\"";
          sweights.insert(sWeight);
        } else {
          dot_file << "\"" << ", penwidth=0.5 color=\"black\"";
        }
      }
    }
    dot_file << "]" << std::endl;
  }
  dot_file << "}";
}

template<class ParentDecorator>
struct print_parent {
  print_parent(const ParentDecorator& p_) :
      p(p_) {
  }
  template<class Vertex>
  void operator()(const Vertex& v) const {
    std::cout << "parent[" << v << "] = " << p[v] << std::endl;
  }
  ParentDecorator p;
};

template<class NewGraph, class Tag>
struct graph_copier: public boost::base_visitor<graph_copier<NewGraph, Tag> > {
  typedef Tag event_filter;

  graph_copier(NewGraph& graph) :
      new_g(graph) {
  }

  template<class Arc, class Graph>
  void operator()(Arc e, Graph& g) {
    boost::add_edge(boost::source(e, g), boost::target(e, g), new_g);
  }
private:
  NewGraph& new_g;
};

template<class NewGraph, class Tag>
inline graph_copier<NewGraph, Tag> copy_graph(NewGraph& g, Tag) {
  return graph_copier<NewGraph, Tag>(g);
}

//==============================================================================
// BFS
//==============================================================================
//TODO: NOT YET WORKING...
void Planner::SolveBFS(Route& route) {
  std::cout << std::endl << "Calculating route using 'BFS2' algorithm for: "
      << route << std::endl;

  typedef boost::adjacency_list<boost::mapS, boost::vecS, boost::bidirectionalS,
      boost::property<boost::vertex_color_t, boost::default_color_type,
          boost::property<boost::vertex_degree_t, int,
              boost::property<boost::vertex_in_degree_t, int,
                  boost::property<boost::vertex_out_degree_t, int> > > > > Graph;

  typedef Graph::vertex_descriptor Vertex;

  int num_nodes = 0;

  //add vertices
  std::set<std::string> vset;
  std::map<std::string, int> vertexNameMap;
  std::map<int, std::string> vrmap;
  std::map<std::string, int> wnmap;
  std::map<int, double> wvmap;

  int idx = 0;
  std::vector<Road> rds = roads->get();
  for (size_t i = 0; i < rds.size(); ++i) {
    std::vector<Edge> edges = rds[i].getEdges();
    for (size_t j = 0; j < edges.size(); ++j) {
      std::string v1 = edges[j].getRoadA()
          + lexical_cast<std::string>(edges[j].getPointA());
      const bool found1 = vset.find(v1) != vset.end();
      if (!found1) {
        //std::cout << "idx " << idx << " vertix 1:\t" << v1 << std::endl;
        vset.insert(v1);
        vertexNameMap[v1] = idx;
        vrmap[idx] = v1;
        idx++;
      }
      std::string v2 = edges[j].getRoadB()
          + lexical_cast<std::string>(edges[j].getPointB());
      const bool found2 = vset.find(v2) != vset.end();
      if (!found2) {
        //std::cout << "idx " << idx << " vertix 2:\t" << v2 << std::endl;
        vset.insert(v2);
        vertexNameMap[v2] = idx;
        vrmap[idx] = v2;
        idx++;
      }
    }
  }
  if (debug)
    std::cout << "Found: " << idx << " vertices" << std::endl;
  vset.clear();
  num_nodes = idx;
  Graph g(num_nodes);

  idx = 0;
  //add edges
  for (size_t i = 0; i < rds.size(); ++i) {
    std::cout << "Adding edges for Road: " << rds[i].getName() << std::endl;
    std::vector<Edge> edges = rds[i].getEdges();
    for (size_t j = 0; j < edges.size(); ++j) {

      std::string v1 = edges[j].getRoadA()
          + lexical_cast<std::string>(edges[j].getPointA());
      std::string v2 = edges[j].getRoadB()
          + lexical_cast<std::string>(edges[j].getPointB());
      int idx1 = vertexNameMap[v1];
      int idx2 = vertexNameMap[v2];
      boost::add_edge(idx1, idx2, g);
      std::string w = v1 + v2;
      wnmap[w] = idx;
      wvmap[idx] = edges[j].getWeight();
      idx++;
    }
  }

  Graph G_copy(num_nodes);
  // Array to store predecessor (parent) of each vertex. This will be
  // used as a Decorator (actually, its iterator will be).
  std::vector<Vertex> p(boost::num_vertices(g));
  // VC++ version of std::vector has no ::pointer, so
  // I use ::value_type* instead.
  typedef std::vector<Vertex>::value_type* Piter;

  // Array to store distances from the source to each vertex .  We use
  // a built-in array here just for variety. This will also be used as
  // a Decorator.
  boost::graph_traits<Graph>::vertices_size_type d[num_nodes];
  std::fill_n(d, num_nodes, 0);

  // The source vertex
  Vertex s = *(boost::vertices(g).first);
  p[s] = s;
  boost::breadth_first_search(g, s,
      boost::visitor(
          boost::make_bfs_visitor(
              std::make_pair(boost::record_distances(d, boost::on_tree_edge()),
                  std::make_pair(
                      boost::record_predecessors(&p[0], boost::on_tree_edge()),
                      copy_graph(G_copy, boost::on_examine_edge()))))));

  if (debug)
    boost::print_graph(g);

  if (boost::num_vertices(g) < 11) {
    if (debug)
      std::cout << "distances: ";
#ifdef BOOST_OLD_STREAM_ITERATORS
    std::copy(d, d + 5, std::ostream_iterator<int, char>(std::cout, " "));
#else
    std::copy(d, d + 5, std::ostream_iterator<int>(std::cout, " "));
#endif
    if (debug)
      std::cout << std::endl;

    if (debug)
      std::for_each(boost::vertices(g).first, boost::vertices(g).second,
          print_parent<Piter>(&p[0]));
  }

  std::ofstream dot_file("./bfs.dot");
  dot_file << "digraph D {\n" << "  rankdir=LR\n" << "  size=\"555,666\"\n"
      << "  ratio=\"fill\"\n" << "  edge[style=\"bold\"]\n"
      << "  node[shape=\"circle\"]\n";

  graph_traits<Graph>::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    graph_traits<Graph>::edge_descriptor e = *ei;
    graph_traits<Graph>::vertex_descriptor u = source(e, g), v = target(e, g);
    std::string es = vrmap[u] + vrmap[v];
    int idx = wnmap[es];
    double w = wvmap[idx];
    dot_file << vrmap[u] << " -> " << vrmap[v] << "\t[label=\"" << w << "\"";
    //<< "[label=\"" << get(weightmap, e) << "\"";
    if (p[v] == u)
      dot_file << ", color=\"black\"";
    else
      dot_file << ", color=\"grey\"";
    dot_file << "]" << std::endl;
  }
  dot_file << "}";
}
