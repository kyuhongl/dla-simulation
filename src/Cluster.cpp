#include "Cluster.h"

Cluster::Cluster() : m_hash(8.0f) {}

void Cluster::reset() {
    m_nodes.clear();
    m_hash.clear();
    m_extent = 0.f;
}

void Cluster::clear() { reset(); }

void Cluster::addSeed(const glm::vec2& p) {
    ClusterNode seed;
    seed.pos = p;
    seed.parent = -1;
    seed.depth = 0;
    m_nodes.push_back(seed);
    m_hash.insert(p, 0);
    m_extent = std::max(m_extent, glm::length(p));
}

void Cluster::addNode(const glm::vec2& p, int parentIndex) {
    ClusterNode n;
    n.pos = p;
    n.parent = parentIndex;
    n.depth = (parentIndex >= 0 && parentIndex < (int)m_nodes.size())
        ? m_nodes[parentIndex].depth + 1 : 0;
    m_nodes.push_back(n);
    m_hash.insert(p, (int)m_nodes.size() - 1);
    m_extent = std::max(m_extent, glm::length(p));
}

void Cluster::rebuildHash(float cellSize) {
    m_hash.setCellSize(cellSize);
    std::vector<glm::vec2> pts;
    pts.reserve(m_nodes.size());
    for (auto& n : m_nodes) pts.push_back(n.pos);
    m_hash.rebuild(pts);
}

void Cluster::queryNeighbors(const glm::vec2& p, std::vector<int>& out) const {
    m_hash.queryNeighbors(p, out);
}
