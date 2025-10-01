#include "RandomGraphGenerator.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <random>
#include <vector>
#include <unordered_set>

Graph RandomGraphGenerator::generateGraph(std::mt19937_64& r, int size, int numEdges) {
    if (size < 1) {
        throw std::invalid_argument("size must be >= 1");
    }
    if (numEdges < size - 1) {
        throw std::invalid_argument("We need min size-1 edges");
    }
    long long maxDirected = 1LL * size * (size - 1);
    if (1LL * numEdges > maxDirected) {
        throw std::invalid_argument("Too many edges for directed graph without self-loops");
    }

    // 1) Случайная перестановка вершин (нормальный Fisher–Yates)
    std::vector<int> perm(size);
    std::iota(perm.begin(), perm.end(), 0);
    for (int i = size - 1; i > 0; --i) {
        std::uniform_int_distribution<int> jdist(0, i);
        int j = jdist(r);
        std::swap(perm[i], perm[j]);
    }

    const int chainCount = size - 1;
    const int needMore = numEdges - chainCount;
    const size_t oversample = static_cast<size_t>(std::max(needMore / 50, 100000));
    const size_t toGenerate = static_cast<size_t>(needMore) + oversample;

    std::vector<uint64_t> keys(static_cast<size_t>(chainCount) + toGenerate);

    // 2) Цепочка из перестановки (гарантия связности)
    for (int i = 1; i < size; ++i) {
        int u = perm[i - 1];
        int v = perm[i];
        keys[static_cast<size_t>(i - 1)] = pack(static_cast<uint32_t>(u), static_cast<uint32_t>(v));
    }

    unsigned hw = std::thread::hardware_concurrency();
    int threads = hw ? static_cast<int>(hw) : 1;

    const size_t offset = static_cast<size_t>(chainCount);
    uint64_t baseSeed = r(); // базовое зерно для "расщепления"

    // 3) Параллельная генерация дополнительных ребер без петель
    parallelFill(keys, offset, toGenerate, threads, size, baseSeed);

    // 4) Сортировка + дедупликация
    std::sort(keys.begin(), keys.end());
    size_t w = 1;
    for (size_t i = 1; i < keys.size(); ++i) {
        if (keys[i] != keys[i - 1]) keys[w++] = keys[i];
    }
    size_t unique = w;

    // 5) Догенерируем пока не будет достаточно уникальных ребер
    while (unique < static_cast<size_t>(numEdges)) {
        size_t missing = static_cast<size_t>(numEdges) - unique;
        size_t extra = std::max(missing / 2, static_cast<size_t>(10000));
        size_t add = missing + extra;

        std::vector<uint64_t> more(unique + add);
        std::copy(keys.begin(), keys.begin() + unique, more.begin());

        uint64_t baseSeed2 = splitmix64(baseSeed ^ 0xBF58476D1CE4E5B9ULL);
        parallelFill(more, unique, add, threads, size, baseSeed2);

        std::sort(more.begin(), more.end());
        w = 1;
        for (size_t i = 1; i < more.size(); ++i) {
            if (more[i] != more[i - 1]) more[w++] = more[i];
        }
        unique = w;
        keys.swap(more);
    }

    // 6) Убираем перекос:
    //    - подтягиваем ребра цепочки в начало
    //    - частично перемешиваем остаток и берем первые numEdges
    std::vector<uint64_t> chain(static_cast<size_t>(chainCount));
    for (int i = 1; i < size; ++i) {
        chain[static_cast<size_t>(i - 1)] = pack(static_cast<uint32_t>(perm[i - 1]),
                                                 static_cast<uint32_t>(perm[i]));
    }
    std::unordered_set<uint64_t> chainSet(chain.begin(), chain.end());
    size_t p = 0;
    for (size_t i = 0; i < unique && p < chain.size(); ++i) {
        if (chainSet.find(keys[i]) != chainSet.end()) {
            std::swap(keys[p], keys[i]);
            chainSet.erase(keys[p]); // удаляем найденное ребро цепочки
            ++p;
        }
    }
    // p — сколько ребер цепочки подтянули (обычно == chainCount)

    // Частичная перетасовка диапазона [p .. unique-1], выбираем первые numEdges
    uint64_t shufSeed = splitmix64(baseSeed ^ 0x94D049BB133111EBULL);
    std::mt19937_64 shuf(shufSeed);
    for (size_t i = p; i < static_cast<size_t>(numEdges); ++i) {
        std::uniform_int_distribution<size_t> distJ(i, unique - 1);
        size_t j = distJ(shuf);
        std::swap(keys[i], keys[j]);
    }

    // 7) Сборка графа из первых numEdges (теперь без смещения по маленьким индексам)
    Graph g(size);
    for (int i = 0; i < numEdges; ++i) {
        uint64_t key = keys[static_cast<size_t>(i)];
        int u = static_cast<int>(unpackU(key));
        int v = static_cast<int>(unpackV(key));
        g.addEdge(u, v);
    }
    return g;
}

uint64_t RandomGraphGenerator::pack(uint32_t u, uint32_t v) {
    return (static_cast<uint64_t>(u) << 32) | static_cast<uint64_t>(v);
}

uint32_t RandomGraphGenerator::unpackU(uint64_t key) {
    return static_cast<uint32_t>(key >> 32);
}

uint32_t RandomGraphGenerator::unpackV(uint64_t key) {
    return static_cast<uint32_t>(key & 0xFFFFFFFFULL);
}

uint64_t RandomGraphGenerator::splitmix64(uint64_t x) {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

void RandomGraphGenerator::parallelFill(std::vector<uint64_t>& keys,
                                        size_t offset,
                                        size_t count,
                                        int threads,
                                        int size,
                                        uint64_t baseSeed) {
    const size_t chunk = (count + static_cast<size_t>(threads) - 1) / static_cast<size_t>(threads);
    std::vector<std::thread> pool;
    pool.reserve(static_cast<size_t>(threads));

    for (int t = 0; t < threads; ++t) {
        pool.emplace_back([&, t] {
            size_t start = offset + static_cast<size_t>(t) * chunk;
            size_t end = std::min(offset + count, start + chunk);
            if (start >= end) return;

            uint64_t seed = splitmix64(baseSeed + 0x9E3779B97F4A7C15ULL * static_cast<uint64_t>(t));
            std::mt19937_64 rnd(seed);
            std::uniform_int_distribution<int> distU(0, size - 1);
            std::uniform_int_distribution<int> distV(0, size - 2);

            for (size_t i = start; i < end; ++i) {
                int u = distU(rnd);
                int v = distV(rnd);
                if (v >= u) ++v; // исключаем самопетлю
                keys[i] = pack(static_cast<uint32_t>(u), static_cast<uint32_t>(v));
            }
        });
    }
    for (auto& th : pool) th.join();
}