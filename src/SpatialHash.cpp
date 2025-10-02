#include "SpatialHash.h"

SpatialHash::SpatialHash(float cell) : cellSize(cell) {}

void SpatialHash::clear() { grid.clear(); }

void SpatialHash::setCellSize(float s) {
    cellSize = std::max(1.0f, s);
    clear();
}

SpatialHash::Key SpatialHash::toKey(const glm::vec2& p) const {
    return { static_cast<int>(std::floor(p.x / cellSize)),
             static_cast<int>(std::floor(p.y / cellSize)) };
}

void SpatialHash::insert(const glm::vec2& p, int index) {
    grid[toKey(p)].push_back(index);
}

void SpatialHash::rebuild(const std::vector<glm::vec2>& points) {
    clear();
    for (int i = 0; i < (int)points.size(); ++i) insert(points[i], i);
}

void SpatialHash::queryNeighbors(const glm::vec2& p, std::vector<int>& out) const {
    out.clear();
    Key k = toKey(p);
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            Key kk{ k.x + dx, k.y + dy };
            auto it = grid.find(kk);
            if (it != grid.end()) {
                const auto& bucket = it->second;
                out.insert(out.end(), bucket.begin(), bucket.end());
            }
        }
    }
}
