#include <vector>
#include <queue>
#include <atomic>
#include <thread>

class Graph {
private:
    int V;
    std::vector<std::vector<int>> adjList;

public:
    Graph(int vertices) : V(vertices), adjList(vertices) {}

    void addEdge(int src, int dest) {
        if (std::find(adjList[src].begin(), adjList[src].end(), dest) == adjList[src].end()) {
            adjList[src].push_back(dest);
        }
    }

    void parallelBFS(int startVertex) {
        // Реализация parallelBFS будет добавлена позже
    }

    void bfs(int startVertex) {
        std::vector<bool> visited(V, false);
        std::queue<int> queue;

        visited[startVertex] = true;
        queue.push(startVertex);

        while (!queue.empty()) {
            startVertex = queue.front();
            queue.pop();

            for (int n : adjList[startVertex]) {
                if (!visited[n]) {
                    visited[n] = true;
                    queue.push(n);
                }
            }
        }
    }
};