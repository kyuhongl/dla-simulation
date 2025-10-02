#pragma once
#include "ofMain.h"

struct Particle {
    glm::vec2 pos;
    glm::vec2 prevPos;
    bool active = true;

    Particle() = default;
    explicit Particle(const glm::vec2& p) : pos(p), prevPos(p) {}
};
