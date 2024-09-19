#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include "Graph.h"

class BFSTest {
private:
    std::vector<int> sizes = {10, 100, 1000, 10000, 10000, 50000, 100000};
    std::vector<int> connections = {50, 500, 5000, 50000, 100000, 1000000, 1000000};
    std::mt19937 rng;

    Graph generateGraph(int size, int numOfConnections) {
        Graph graph(size);
        std::uniform_int_distribution<int> dist(0, size - 1);
        for (int i = 0; i < numOfConnections; i++) {
            graph.addEdge(dist(rng), dist(rng));
        }
        return graph;
    }

    long long executeSerialBfsAndGetTime(Graph& g) {
        auto startTime = std::chrono::high_resolution_clock::now();
        g.bfs(0);
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    }

    long long executeParallelBfsAndGetTime(Graph& g) {
        auto startTime = std::chrono::high_resolution_clock::now();
        g.parallelBFS(0);
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    }

public:
    BFSTest() : rng(42) {} // Инициализация генератора случайных чисел с сидом 42

    void runTests() {
        for (size_t i = 0; i < sizes.size(); i++) {
            Graph g = generateGraph(sizes[i], connections[i]);
            long long serialTime = executeSerialBfsAndGetTime(g);
            long long parallelTime = executeParallelBfsAndGetTime(g);
            
            std::cout << "Times for " << sizes[i] << " vertices and " << connections[i] << " connections: \n";
            std::cout << "Serial: " << serialTime << " ms\n";
            std::cout << "Parallel: " << parallelTime << " ms\n";
            std::cout << "--------\n";
        }
    }
};

int main() {
    BFSTest test;
    test.runTests();
    return 0;
}