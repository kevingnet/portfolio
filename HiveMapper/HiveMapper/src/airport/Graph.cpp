/*
 * Graph.cpp
 *
 *  Created on: Jan 4, 2018
 *
 *  Credits" https://www.geeksforgeeks.org/depth-first-traversal-for-a-graph/
 */

#include "Graph.h"

using namespace std;

void Graph::BFS(int s, int d) {
  bool *visited = new bool[verticesCount];
  for (int i = 0; i < verticesCount; i++)
    visited[i] = false;

  list<int> queue;

  visited[s] = true;
  queue.push_back(s);

  list<int>::iterator it;

  int prev = s;
  bool skipFirst = true;

  while (!queue.empty()) {
    // Dequeue a vertex from queue and print it
    s = queue.front();
    if (!skipFirst) {
      result.push_back(std::make_pair(prev, s));
      cout << prev << " " << s << "\n";
    } else {
      skipFirst = false;
    }
    prev = s;
    //string name = vertexIndexes[s];
    //cout << s << " " << name << "\n";
    queue.pop_front();
    if (s == d)
      break;

    // Get all adjacent vertices of the dequeued
    // vertex s. If a adjacent has not been visited,
    // then mark it visited and enqueue it
    for (it = vertices[s].begin(); it != vertices[s].end(); ++it) {
      if (!visited[*it]) {
        visited[*it] = true;
        queue.push_back(*it);
      }
    }
  }
  delete[] visited;
}

void Graph::DFSUtil(int v, int d, bool visited[]) {
  if (v == d)
    return;
  visited[v] = true;
  cout << v << " ";

  list<int>::iterator i;
  for (i = vertices[v].begin(); i != vertices[v].end(); ++i)
    if (!visited[*i])
      DFSUtil(*i, d, visited);
}

void Graph::DFS(int v, int d) {
  bool *visited = new bool[verticesCount];
  for (int i = 0; i < verticesCount; i++)
    visited[i] = false;

  DFSUtil(v, d, visited);
}

void Graph::allPaths(int s, int d) {
  // Mark all the vertices as not visited
  bool *visited = new bool[verticesCount];

  // Create an array to store paths
  int *path = new int[verticesCount];
  int path_index = 0; // Initialize path[] as empty

  // Initialize all vertices as not visited
  for (int i = 0; i < verticesCount; i++)
    visited[i] = false;

  // Call the recursive helper function to print all paths
  allPathsUtil(s, d, visited, path, path_index);
}

// A recursive function to print all paths from 'u' to 'd'.
// visited[] keeps track of vertices in current path.
// path[] stores actual vertices and path_index is current
// index in path[]
void Graph::allPathsUtil(int u, int d, bool visited[], int path[],
    int &path_index) {
  // Mark the current node and store it in path[]
  visited[u] = true;
  path[path_index] = u;
  path_index++;


  // If current vertex is same as destination, then print
  // current path[]
  if (u == d) {
    int prev = path[0];
    allPathsCurrent.clear();
    for (int i = 1; i < path_index; i++) {
      allPathsCurrent.push_back(std::make_pair(prev, path[i]));
      prev = path[i];
    }
    allPathsResult.push_back(allPathsCurrent);
  } else // If current vertex is not destination
  {
    // Recur for all the vertices adjacent to current vertex
    list<int>::iterator i;
    for (i = vertices[u].begin(); i != vertices[u].end(); ++i)
      if (!visited[*i])
        allPathsUtil(*i, d, visited, path, path_index);
  }

  // Remove current vertex from path[] and mark it as unvisited
  path_index--;
  visited[u] = false;
}
