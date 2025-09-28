#include "Graph.h"
#include <algorithm>
#include <queue>

Graph::Graph(int vertices) : V(vertices), adjList(vertices) {}

void Graph::addEdge(int src, int dest) {
    if (src < 0 || dest < 0 || src >= V || dest >= V) return;
    auto& vec = adjList[src];
    if (std::find(vec.begin(), vec.end(), dest) == vec.end()) {
        vec.push_back(dest);
    }
}

void Graph::parallelBFS(int /*startVertex*/) {
    // Заглушка, как в Java-версии
}

void Graph::bfs(int startVertex) {
    if (startVertex < 0 || startVertex >= V) return;
    std::vector<char> visited(V, 0);
    std::queue<int> q;

    visited[startVertex] = 1;
    q.push(startVertex);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int n : adjList[u]) {
            if (!visited[n]) {
                visited[n] = 1;
                q.push(n);
            }
        }
    }
}

int Graph::vertices() const { return V; }