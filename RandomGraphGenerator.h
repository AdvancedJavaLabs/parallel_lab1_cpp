#pragma once
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
#include "Graph.h"

class RandomGraphGenerator {
public:
    Graph generateGraph(std::mt19937_64& r, int size, int numEdges);

private:
    static uint64_t pack(uint32_t u, uint32_t v);
    static uint32_t unpackU(uint64_t key);
    static uint32_t unpackV(uint64_t key);
    static uint64_t splitmix64(uint64_t x);
    static void parallelFill(std::vector<uint64_t>& keys,
                             size_t offset,
                             size_t count,
                             int threads,
                             int size,
                             uint64_t baseSeed);
};