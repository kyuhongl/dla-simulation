#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "Particle.h"
#include "Cluster.h"
#include <random>

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;

    void keyPressed(int key) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void windowResized(int w, int h) override;

private:
    // Simulation
    Cluster cluster;
    std::vector<Particle> walkers;

    // RNG
    std::mt19937 rng;
    float rand01();

    // Params (GUI)
    ofxPanel gui;
    ofParameter<int> numWalkers;
    ofParameter<float> stickRadius;
    ofParameter<float> stepSize;
    ofParameter<float> stickProb;
    ofParameter<float> spawnMargin;
    ofParameter<float> killMargin;
    ofParameter<int> maxStuck;
    ofParameter<uint32_t> seedParam;
    ofParameter<bool> deterministic;
    ofParameter<bool> drawLines;
    ofParameter<bool> drawPoints;
    ofParameter<bool> drawWalkers;
    ofParameter<bool> fadeTrails;
    ofParameter<bool> autoPauseOnMax;

    // NEW: performance controls
    ofParameter<int> frameBudgetMs;      // per-frame CPU budget for stepping walkers
    ofParameter<int> drawMaxNodes;       // max nodes to draw each frame before decimating
    ofParameter<bool> perfSafeMode;      // enable budgets/decimation

    // State
    bool paused = false;
    float zoom = 1.0f;
    float spawnRadius = 80.f;
    float killRadius = 160.f;

    // Helpers
    void initRNG();
    void resetSim();
    void ensureWalkerCount();
    glm::vec2 randomPointOnRing(float radius);
    void respawnWalker(Particle& w);
    bool stepWalker(Particle& w); // returns true if stuck this frame
    bool tryStick(const glm::vec2& pos, int& outParentIdx, float& outNearestDistSq);
    void updateRadii();
    void drawScene();
    void exportPNG() const;

    // Spatial hash rebuild management
    float lastCellSize = -1.f;

    // Time-budgeted stepping
    size_t walkerStart = 0; // rotating index across frames

    // Cached query buffer
    std::vector<int> neighborCandidates;
    
    // Shaders
    ofShader testShader;
    ofShader backgroundShader;
    bool shaderLoaded = false;
    bool shaderEnabled = true;
    bool backgroundShaderLoaded = false;
    
    // GIF recording
    bool isRecordingGif = false;
    int gifFrameCount = 0;
    int gifTotalFrames = 90; // 3 seconds at 30fps
    std::string gifFolderPath;
    void startGifRecording();
    void updateGifRecording();
    void finishGifRecording();
};