#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <queue>
#include <atomic>
#include <thread>

class Graph {
private:
    int V;
    std::vector<std::vector<int>> adjList;

public:
    Graph(int vertices);
    
    void addEdge(int src, int dest);
    
    void parallelBFS(int startVertex);
    
    void bfs(int startVertex);
};

#endif // GRAPH_H