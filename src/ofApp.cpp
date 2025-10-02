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
    
    // Random angle
    float angle = rand01() * TWO_PI;
    
    // Random radius with bias towards spawn margin
    // Use power distribution: higher power = more clustering at the edge
    float t = rand01();
    float radiusBias = std::pow(t, 2.0f); // square for bias towards outer edge
    
    // Spawn in a range from inner radius to outer radius
    float minRadius = spawnRadius * 0.5f; // can spawn from 50% to 100% of spawn radius
    float maxRadius = spawnRadius * 1.5f; // up to 150% for more spread
    float actualRadius = minRadius + radiusBias * (maxRadius - minRadius);
    
    w.pos = glm::vec2(actualRadius * std::cos(angle), actualRadius * std::sin(angle));
    w.prevPos = w.pos;
}

void ofApp::updateRadii() {
    float ext = std::max(cluster.extent(), 1.f);
    spawnRadius = ext + spawnMargin.get() * 1.5f; // increase spawn radius
    killRadius  = ext + (spawnMargin.get() * 2.0f + killMargin.get()); // adjust kill radius accordingly
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
    ofBackground(18, 25, 38); // dark navy blue

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
    gui.add(drawWalkers.set("drawWalkers", true));
    gui.add(fadeTrails.set("fadeTrails", true));
    gui.add(autoPauseOnMax.set("autoPauseOnMax", true));

    // NEW: performance controls
    gui.add(perfSafeMode.set("perfSafeMode", true));
    gui.add(frameBudgetMs.set("frameBudgetMs", 6, 0, 16));   // ~6ms simulation per frame
    gui.add(drawMaxNodes.set("drawMaxNodes", 12000, 2000, 60000));

    // Load shaders
    shaderLoaded = testShader.load("shaders/simple_test");
    if (shaderLoaded) {
        ofLogNotice() << "Shader loaded successfully";
    } else {
        ofLogError() << "Failed to load shader";
    }
    
    backgroundShaderLoaded = backgroundShader.load("shaders/background");
    if (backgroundShaderLoaded) {
        ofLogNotice() << "Background shader loaded successfully";
    } else {
        ofLogError() << "Failed to load background shader";
    }

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
    // Draw wispy background shader (before transformations)
    if (backgroundShaderLoaded) {
        ofPushMatrix();
        ofPushStyle();
        backgroundShader.begin();
        backgroundShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
        backgroundShader.setUniform1f("time", ofGetElapsedTimef());
        ofSetColor(255);
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
        backgroundShader.end();
        ofPopStyle();
        ofPopMatrix();
    } else {
        ofBackground(18, 25, 38); // fallback to solid navy blue
    }
    
    ofPushMatrix();
    ofTranslate(ofGetWidth()*0.5f, ofGetHeight()*0.5f);
    ofScale(zoom, zoom);

    if (fadeTrails) {
        ofPushStyle();
        ofSetColor(0, 0, 0, 10); // darker trail fade for wispy background
        ofDrawRectangle(-ofGetWidth(), -ofGetHeight(), ofGetWidth()*2, ofGetHeight()*2);
        ofPopStyle();
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
        ofEnableBlendMode(OF_BLENDMODE_ADD); // additive blending for glow
        
        // Batch all lines into a mesh for efficient rendering - use triangles for thickness variation
        ofMesh linesMesh;
        linesMesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        if (stride == 1) {
            for (int k = 0; k < N; ++k) {
                const auto& n = nodes[k];
                if (n.parent >= 0) {
                    const auto& p = nodes[n.parent];
                    
                    // Calculate line properties
                    glm::vec2 dir = n.pos - p.pos;
                    float lineLength = glm::length(dir);
                    if (lineLength < 0.01f) continue;
                    
                    dir = glm::normalize(dir);
                    glm::vec2 perpendicular(-dir.y, dir.x);
                    
                    // Vary thickness based on depth (deeper = thicker)
                    float baseThickness = 1.2f + n.depth * 0.003f;
                    baseThickness = std::min(baseThickness, 2.5f);
                    
                    // Create curved line with multiple segments for organic look
                    int segments = std::max(2, (int)(lineLength / 12.0f));
                    segments = std::min(segments, 5); // cap for performance
                    
                    for (int seg = 0; seg < segments; seg++) {
                        float t1 = seg / (float)segments;
                        float t2 = (seg + 1) / (float)segments;
                        
                        // Interpolate positions
                        glm::vec2 pos1 = p.pos + dir * (lineLength * t1);
                        glm::vec2 pos2 = p.pos + dir * (lineLength * t2);
                        
                        // Add very subtle organic wave displacement (reduced)
                        float waveFreq = 0.2f + (k % 10) * 0.03f; // vary per line
                        float wave1 = sin(t1 * 6.28f * waveFreq + k * 0.1f) * lineLength * 0.03f;
                        float wave2 = sin(t2 * 6.28f * waveFreq + k * 0.1f) * lineLength * 0.03f;
                        
                        pos1 += perpendicular * wave1;
                        pos2 += perpendicular * wave2;
                        
                        // Thickness taper (thinner at child end) - less taper for consistency
                        float thickness1 = baseThickness * (0.8f + t1 * 0.2f);
                        float thickness2 = baseThickness * (0.8f + t2 * 0.2f);
                        
                        // Color variation along line
                        int alpha1 = ofClamp(40 + n.depth + (int)(t1 * 40), 40, 180);
                        int alpha2 = ofClamp(40 + n.depth + (int)(t2 * 40), 40, 180);
                        ofColor color1(255, 255, 255, alpha1);
                        ofColor color2(255, 255, 255, alpha2);
                        
                        // Create quad as two triangles
                        glm::vec2 newPerp = glm::normalize(glm::vec2(-(pos2.y - pos1.y), pos2.x - pos1.x));
                        
                        // Triangle 1
                        linesMesh.addVertex(glm::vec3(pos1 + newPerp * thickness1, 0));
                        linesMesh.addColor(color1);
                        linesMesh.addTexCoord(glm::vec2(t1, 0.0)); // flowing texture coord
                        
                        linesMesh.addVertex(glm::vec3(pos1 - newPerp * thickness1, 0));
                        linesMesh.addColor(color1);
                        linesMesh.addTexCoord(glm::vec2(t1, 1.0));
                        
                        linesMesh.addVertex(glm::vec3(pos2 + newPerp * thickness2, 0));
                        linesMesh.addColor(color2);
                        linesMesh.addTexCoord(glm::vec2(t2, 0.0));
                        
                        // Triangle 2
                        linesMesh.addVertex(glm::vec3(pos2 + newPerp * thickness2, 0));
                        linesMesh.addColor(color2);
                        linesMesh.addTexCoord(glm::vec2(t2, 0.0));
                        
                        linesMesh.addVertex(glm::vec3(pos1 - newPerp * thickness1, 0));
                        linesMesh.addColor(color1);
                        linesMesh.addTexCoord(glm::vec2(t1, 1.0));
                        
                        linesMesh.addVertex(glm::vec3(pos2 - newPerp * thickness2, 0));
                        linesMesh.addColor(color2);
                        linesMesh.addTexCoord(glm::vec2(t2, 1.0));
                    }
                }
            }
        } else {
            // Lightweight fallback: simpler straight lines when stride > 1
            ofColor sparseColor(255, 255, 255, 60);
            for (int k = 0; k < N; k += stride) {
                const auto& n = nodes[k];
                if (n.parent >= 0) {
                    const auto& p = nodes[n.parent];
                    glm::vec2 dir = glm::normalize(n.pos - p.pos);
                    glm::vec2 perp(-dir.y, dir.x);
                    float thickness = 1.2f;
                    
                    linesMesh.addVertex(glm::vec3(n.pos + perp * thickness, 0));
                    linesMesh.addColor(sparseColor);
                    linesMesh.addTexCoord(glm::vec2(1.0, 0.0));
                    
                    linesMesh.addVertex(glm::vec3(n.pos - perp * thickness, 0));
                    linesMesh.addColor(sparseColor);
                    linesMesh.addTexCoord(glm::vec2(1.0, 1.0));
                    
                    linesMesh.addVertex(glm::vec3(p.pos + perp * thickness, 0));
                    linesMesh.addColor(sparseColor);
                    linesMesh.addTexCoord(glm::vec2(0.0, 0.0));
                    
                    linesMesh.addVertex(glm::vec3(p.pos + perp * thickness, 0));
                    linesMesh.addColor(sparseColor);
                    linesMesh.addTexCoord(glm::vec2(0.0, 0.0));
                    
                    linesMesh.addVertex(glm::vec3(n.pos - perp * thickness, 0));
                    linesMesh.addColor(sparseColor);
                    linesMesh.addTexCoord(glm::vec2(1.0, 1.0));
                    
                    linesMesh.addVertex(glm::vec3(p.pos - perp * thickness, 0));
                    linesMesh.addColor(sparseColor);
                    linesMesh.addTexCoord(glm::vec2(0.0, 1.0));
                }
            }
        }
        
        // Apply shader and draw all lines in one batch
        if (shaderLoaded && shaderEnabled) {
            testShader.begin();
            testShader.setUniform1f("time", ofGetElapsedTimef());
            linesMesh.draw();
            testShader.end();
        } else {
            linesMesh.draw();
        }
        
        ofDisableBlendMode();
        ofPopStyle();
    }

    if (drawPoints) {
        ofPushStyle();
        ofSetColor(255);
        ofFill();
        
        // Batch draw circles using mesh for better performance
        ofMesh pointsMesh;
        pointsMesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        const int circleResolution = 12; // slightly higher for smoother spheres
        
        for (int k = 0; k < N; k += stride) {
            const auto& node = nodes[k];
            const auto& pos = node.pos;
            
            // Vary particle size based on depth (older = slightly larger)
            // Make particles bigger to connect better with lines
            float depthFactor = std::min(1.0f + node.depth * 0.003f, 1.8f);
            float radius = 2.5f * depthFactor;
            
            // Vary color intensity based on depth
            float colorIntensity = ofClamp(200 + node.depth * 0.5f, 200, 255);
            ofColor particleColor(colorIntensity);
            
            // Create a circle as triangle fan with texture coordinates
            for (int i = 0; i < circleResolution; i++) {
                float angle1 = (i / (float)circleResolution) * TWO_PI;
                float angle2 = ((i + 1) / (float)circleResolution) * TWO_PI;
                
                float x1 = cos(angle1);
                float y1 = sin(angle1);
                float x2 = cos(angle2);
                float y2 = sin(angle2);
                
                // Center vertex
                pointsMesh.addVertex(glm::vec3(pos, 0));
                pointsMesh.addColor(particleColor);
                pointsMesh.addTexCoord(glm::vec2(0.5, 0.5)); // center of circle
                
                // Edge vertex 1
                pointsMesh.addVertex(glm::vec3(pos.x + x1 * radius, pos.y + y1 * radius, 0));
                pointsMesh.addColor(particleColor);
                pointsMesh.addTexCoord(glm::vec2(0.5 + x1 * 0.5, 0.5 + y1 * 0.5)); // edge
                
                // Edge vertex 2
                pointsMesh.addVertex(glm::vec3(pos.x + x2 * radius, pos.y + y2 * radius, 0));
                pointsMesh.addColor(particleColor);
                pointsMesh.addTexCoord(glm::vec2(0.5 + x2 * 0.5, 0.5 + y2 * 0.5)); // edge
            }
        }
        
        // Apply shader and draw all points in one batch
        if (shaderLoaded && shaderEnabled) {
            testShader.begin();
            testShader.setUniform1f("time", ofGetElapsedTimef());
            pointsMesh.draw();
            testShader.end();
        } else {
            pointsMesh.draw();
        }
        ofPopStyle();
    }

    // walkers (optional)
    if (drawWalkers) {
        ofPushStyle();
        
        // Apply shader only to walkers if enabled
        bool applyShader = shaderLoaded && shaderEnabled && N < 10000; // disable shader when cluster is huge
        if (applyShader) {
            testShader.begin();
            testShader.setUniform1f("time", ofGetElapsedTimef());
        } else {
            ofSetColor(255, 255, 255, 80);
        }
        
        int maxWalkersToDraw = perfSafeMode ? std::min((int)walkers.size(), 1000) : (int)walkers.size();
        for (int w = 0; w < maxWalkersToDraw; ++w) {
            const auto& pos = walkers[w].pos;
            ofDrawCircle(pos, 1.0f);
        }
        
        if (applyShader) {
            testShader.end();
        }
        
        ofPopStyle();
    }

    ofPopMatrix();
}

void ofApp::draw() {
    drawScene();
    
    // Update GIF recording if active
    if (isRecordingGif) {
        updateGifRecording();
    }
    
    ofPushStyle();
    ofDisableDepthTest();
    gui.draw();
    
    // Show recording indicator
    if (isRecordingGif) {
        ofSetColor(255, 50, 50);
        ofDrawCircle(ofGetWidth() - 30, 30, 10);
        ofSetColor(255);
        ofDrawBitmapString("Recording GIF: " + ofToString(gifFrameCount) + "/" + ofToString(gifTotalFrames), 
                          ofGetWidth() - 200, 50);
    }
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
        case 'g': startGifRecording(); break; // Start GIF recording
        case 's': deterministic = !deterministic; resetSim(); break;
        case 'l': drawLines = !drawLines; break;
        case 'p': drawPoints = !drawPoints; break;
        case 'w': drawWalkers = !drawWalkers; break;
        case 'f': fadeTrails = !fadeTrails; break;
        case 'h': shaderEnabled = !shaderEnabled; break; // toggle shader
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

// GIF Recording Functions
void ofApp::startGifRecording() {
    if (isRecordingGif) {
        ofLogWarning() << "Already recording GIF";
        return;
    }
    
    // Create unique folder for this GIF
    auto timestamp = ofGetTimestampString("%Y%m%d_%H%M%S");
    gifFolderPath = "gif_frames_" + timestamp;
    ofDirectory::createDirectory(gifFolderPath);
    
    isRecordingGif = true;
    gifFrameCount = 0;
    
    ofLogNotice() << "Started GIF recording to: " << gifFolderPath;
}

void ofApp::updateGifRecording() {
    if (!isRecordingGif) return;
    
    // Capture current frame
    ofImage frameImage;
    frameImage.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
    
    // Save frame with zero-padded number
    std::string filename = gifFolderPath + "/frame_" + ofToString(gifFrameCount, 4, '0') + ".png";
    frameImage.save(filename);
    
    gifFrameCount++;
    
    // Check if we've captured enough frames
    if (gifFrameCount >= gifTotalFrames) {
        finishGifRecording();
    }
}

void ofApp::finishGifRecording() {
    isRecordingGif = false;
    ofLogNotice() << "Finished GIF recording! Saved " << gifFrameCount << " frames to: " << gifFolderPath;
    ofLogNotice() << "To create GIF, run: ffmpeg -i " << gifFolderPath << "/frame_%04d.png -vf \"fps=30,scale=800:-1:flags=lanczos\" output.gif";
    ofLogNotice() << "Or use ImageMagick: convert -delay 3.33 -loop 0 " << gifFolderPath << "/frame_*.png output.gif";
}