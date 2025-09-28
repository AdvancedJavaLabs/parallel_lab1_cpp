#pragma once
#include <vector>

class Graph {
public:
    explicit Graph(int vertices);
    void addEdge(int src, int dest);
    void parallelBFS(int startVertex); // заглушка, как в Java
    void bfs(int startVertex);         // обычный BFS
    int vertices() const;

private:
    int V;
    std::vector<std::vector<int>> adjList;
};