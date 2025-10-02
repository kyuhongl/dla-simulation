#pragma once
#include "ofMain.h"
#include <unordered_map>
#include <vector>

class SpatialHash {
public:
    explicit SpatialHash(float cellSize = 8.0f);

    void clear();
    void rebuild(const std::vector<glm::vec2>& points);
    void insert(const glm::vec2& p, int index);
    void setCellSize(float s);
    float getCellSize() const { return cellSize; }

    // Return candidate neighbor indices (cluster point indices)
    void queryNeighbors(const glm::vec2& p, std::vector<int>& out) const;

private:
    struct Key {
        int x, y;
        bool operator==(const Key& o) const { return x == o.x && y == o.y; }
    };
    struct KeyHasher {
        size_t operator()(const Key& k) const {
            // 64-bit mix
            return (std::hash<int>()(k.x) * 73856093u) ^ (std::hash<int>()(k.y) * 19349663u);
        }
    };

    Key toKey(const glm::vec2& p) const;
    float cellSize;
    std::unordered_map<Key, std::vector<int>, KeyHasher> grid;
};
