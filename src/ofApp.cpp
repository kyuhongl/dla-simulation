#include "ofApp.h"
#include <limits>

// ---------------- RNG helpers ----------------
float ofApp::rand01() { return std::generate_canonical<float, 24>(rng); }

void ofApp::initRNG() {
    if (deterministic) rng.seed(seedParam.get());
    else rng.seed((uint32_t)ofGetElapsedTimeMicros());
}

glm::vec2 ofApp::randomPointOnRing(float radius) {
    float a = rand01() * TWO_PI;
    return { radius * std::cos(a), radius * std::sin(a) };
}

void ofApp::respawnWalker(Particle& w) {
    w.active = true;
    w.pos = randomPointOnRing(spawnRadius);
    w.prevPos = w.pos;
}

void ofApp::updateRadii() {
    float ext = std::max(cluster.extent(), 1.f);
    spawnRadius = ext + spawnMargin.get();
    killRadius  = ext + (spawnMargin.get() + killMargin.get());
}

// pos is where the walker is; find nearest cluster node and decide if it sticks
bool ofApp::tryStick(const glm::vec2& pos, int& outParentIdx, float& outNearestDistSq) {
    outParentIdx = -1;
    outNearestDistSq = std::numeric_limits<float>::max();

    cluster.queryNeighbors(pos, neighborCandidates);
    const auto& nodes = cluster.nodes();

    float r2 = stickRadius.get() * stickRadius.get();
    for (int idx : neighborCandidates) {
        const glm::vec2& c = nodes[idx].pos;
        float d2 = glm::length2(c - pos);
        if (d2 < outNearestDistSq) {
            outNearestDistSq = d2;
            outParentIdx = idx;
        }
    }
    // Within threshold and passes probability?
    if (outNearestDistSq <= r2 && rand01() <= stickProb.get()) return true;
    return false;
}

bool ofApp::stepWalker(Particle& w) {
    // random unit step
    float a = rand01() * TWO_PI;
    glm::vec2 step = glm::vec2(std::cos(a), std::sin(a)) * stepSize.get();
    w.prevPos = w.pos;
    w.pos += step;

    float r = glm::length(w.pos);
    if (r > killRadius) {
        respawnWalker(w);
        return false;
    }

    int parentIdx;
    float nearestSq;
    if (tryStick(w.pos, parentIdx, nearestSq)) {
        cluster.addNode(w.pos, parentIdx); // incrementally updates spatial hash
        updateRadii();
        respawnWalker(w);
        return true;
    }
    return false;
}

void ofApp::ensureWalkerCount() {
    if ((int)walkers.size() < numWalkers.get()) {
        int need = numWalkers.get() - (int)walkers.size();
        walkers.reserve(numWalkers.get());
        for (int i = 0; i < need; ++i) {
            Particle w;
            respawnWalker(w);
            walkers.push_back(w);
        }
    } else if ((int)walkers.size() > numWalkers.get()) {
        walkers.resize(numWalkers.get());
        walkerStart %= walkers.size();
    }
}

void ofApp::resetSim() {
    initRNG();
    cluster.reset();
    cluster.addSeed({0,0});
    updateRadii();
    walkers.clear();
    ensureWalkerCount();

    // Set spatial hash cell size once (rebuild only if cell size changes later)
    lastCellSize = std::max(stickRadius.get() * 2.f, stepSize.get() * 2.f);
    cluster.rebuildHash(lastCellSize);
}

// ---------------- oF lifecycle ----------------
void ofApp::setup() {
    ofSetWindowTitle("DLA — openFrameworks");
    ofSetFrameRate(60);
    ofBackground(0);

    gui.setup("DLA");
    gui.add(numWalkers.set("numWalkers", 1024, 32, 8192));
    gui.add(stickRadius.set("stickRadius", 3.0f, 0.5f, 12.0f));
    gui.add(stepSize.set("stepSize", 2.0f, 0.25f, 8.0f));
    gui.add(stickProb.set("stickProb", 1.0f, 0.0f, 1.0f));
    gui.add(spawnMargin.set("spawnMargin", 40.0f, 4.0f, 200.0f));
    gui.add(killMargin.set("killMargin", 120.0f, 20.0f, 400.0f));
    gui.add(maxStuck.set("maxStuck", 20000, 100, 200000));
    gui.add(seedParam.set("seed", 1337));
    gui.add(deterministic.set("deterministic", true));
    gui.add(drawLines.set("drawLines", true));
    gui.add(drawPoints.set("drawPoints", true));
    gui.add(fadeTrails.set("fadeTrails", true));
    gui.add(autoPauseOnMax.set("autoPauseOnMax", true));

    // NEW: performance controls
    gui.add(perfSafeMode.set("perfSafeMode", true));
    gui.add(frameBudgetMs.set("frameBudgetMs", 6, 0, 16));   // ~6ms simulation per frame
    gui.add(drawMaxNodes.set("drawMaxNodes", 12000, 2000, 60000));

    resetSim();
}

void ofApp::update() {
    if (paused) return;

    // Only rebuild spatial hash when cell size changes (e.g., on parameter tweaks)
    float wantedCell = std::max(stickRadius.get() * 2.f, stepSize.get() * 2.f);
    if (std::abs(wantedCell - lastCellSize) > 0.01f) {
        lastCellSize = wantedCell;
        cluster.rebuildHash(lastCellSize);
    }

    ensureWalkerCount();

    // Time-budgeted stepping: spread work across frames
    const auto start = ofGetElapsedTimeMicros();
    const uint64_t budgetUs = (perfSafeMode && frameBudgetMs.get() > 0)
        ? (uint64_t)frameBudgetMs.get() * 1000ull
        : std::numeric_limits<uint64_t>::max();

    int total = (int)walkers.size();
    if (total == 0) return;

    int processed = 0;
    int i = (int)walkerStart;

    while (processed < total) {
        if ((int)cluster.nodes().size() >= maxStuck.get()) break;

        stepWalker(walkers[i]);

        ++processed;
        i = (i + 1) % total;

        if (ofGetElapsedTimeMicros() - start > budgetUs) {
            walkerStart = (size_t)i; // resume next frame
            break;
        }
    }

    if ((int)cluster.nodes().size() >= maxStuck.get() && autoPauseOnMax.get()) paused = true;
}

void ofApp::drawScene() {
    ofPushMatrix();
    ofTranslate(ofGetWidth()*0.5f, ofGetHeight()*0.5f);
    ofScale(zoom, zoom);

    if (fadeTrails) {
        ofPushStyle();
        ofSetColor(0, 0, 0, 18); // subtle trail fade toward black
        ofDrawRectangle(-ofGetWidth(), -ofGetHeight(), ofGetWidth()*2, ofGetHeight()*2);
        ofPopStyle();
    } else {
        ofBackground(0);
    }

    const auto& nodes = cluster.nodes();
    int N = (int)nodes.size();

    // Decimate draw if large
    int stride = 1;
    if (perfSafeMode && N > drawMaxNodes.get()) {
        stride = std::max(1, (int)std::ceil((float)N / drawMaxNodes.get()));
    }

    if (drawLines) {
        ofPushStyle();
        ofSetLineWidth(1.0f);
        if (stride == 1) {
            for (int k = 0; k < N; ++k) {
                const auto& n = nodes[k];
                if (n.parent >= 0) {
                    const auto& p = nodes[n.parent];
                    int a = ofClamp(40 + n.depth, 40, 180);
                    ofSetColor(255, 255, 255, a);
                    ofDrawLine(n.pos, p.pos);
                }
            }
        } else {
            // Lightweight fallback: skip lines when huge, or draw sparse lines
            for (int k = 0; k < N; k += stride) {
                const auto& n = nodes[k];
                if (n.parent >= 0) {
                    const auto& p = nodes[n.parent];
                    ofSetColor(255, 255, 255, 60);
                    ofDrawLine(n.pos, p.pos);
                }
            }
        }
        ofPopStyle();
    }

    if (drawPoints) {
        ofPushStyle();
        ofSetColor(255);
        for (int k = 0; k < N; k += stride) {
            ofDrawCircle(nodes[k].pos, 1.5f);
        }
        ofPopStyle();
    }

    // walkers (optional)
    ofPushStyle();
    ofSetColor(255, 255, 255, 80);
    int maxWalkersToDraw = perfSafeMode ? std::min((int)walkers.size(), 2000) : (int)walkers.size();
    for (int w = 0; w < maxWalkersToDraw; ++w) ofDrawCircle(walkers[w].pos, 1.0f);
    ofPopStyle();

    ofPopMatrix();
}

void ofApp::draw() {
    drawScene();
    ofPushStyle();
    ofDisableDepthTest();
    gui.draw();
    ofPopStyle();
}

void ofApp::exportPNG() const {
    auto ts = ofGetTimestampString("%Y%m%d_%H%M%S");
    ofSaveScreen("DLA_" + ts + ".png");
    ofLogNotice() << "Saved DLA_" << ts << ".png";
}

void ofApp::keyPressed(int key) {
    switch (key) {
        case ' ': paused = !paused; break;
        case 'r': resetSim(); paused = false; break;
        case 'e': exportPNG(); break;
        case 's': deterministic = !deterministic; resetSim(); break;
        case 'l': drawLines = !drawLines; break;
        case 'p': drawPoints = !drawPoints; break;
        case 'f': fadeTrails = !fadeTrails; break;
        case '+': case '=': zoom = std::min(zoom * 1.1f, 100.0f); break;
        case '-': case '_': zoom = std::max(zoom / 1.1f, 0.05f); break;
        case OF_KEY_UP:
            numWalkers = std::min(numWalkers.get() + 64, 8192);
            ensureWalkerCount();
            break;
        case OF_KEY_DOWN:
            numWalkers = std::max(numWalkers.get() - 64, 32);
            ensureWalkerCount();
            break;
    }
}

void ofApp::windowResized(int, int) {
    // no-op; world is centered
}

void ofApp::mouseScrolled(int, int, float, float scrollY) {
    if (scrollY > 0) zoom = std::min(zoom * 1.1f, 100.0f);
    else if (scrollY < 0) zoom = std::max(zoom / 1.1f, 0.05f);
}