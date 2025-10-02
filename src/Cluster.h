#pragma once
#include "ofMain.h"
#include "SpatialHash.h"

struct ClusterNode {
    glm::vec2 pos;
    int parent = -1;  // index of parent node (nearest upon sticking), -1 for seed
    int depth = 0;    // steps from seed
};

class Cluster {
public:
    Cluster();

    void reset();
    void addSeed(const glm::vec2& p);
    // Add node, record parent and depth; updates extent and hash
    void addNode(const glm::vec2& p, int parentIndex);
    void rebuildHash(float cellSize);
    void clear();

    const std::vector<ClusterNode>& nodes() const { return m_nodes; }
    std::vector<ClusterNode>& nodes() { return m_nodes; }

    float extent() const { return m_extent; } // max radius from origin
    glm::vec2 centroid() const { return {0,0}; } // we center world at (0,0)

    // neighbor search (candidate indices)
    void queryNeighbors(const glm::vec2& p, std::vector<int>& out) const;

private:
    std::vector<ClusterNode> m_nodes;
    SpatialHash m_hash;
    float m_extent = 0.f;
};
