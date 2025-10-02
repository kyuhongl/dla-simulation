#include "ofMain.h"
#include "ofApp.h"

int main() {
    ofGLFWWindowSettings settings;
    settings.setGLVersion(3, 2);   // GL3 core -> GLSL #version 150
    settings.setSize(1280, 800);
    ofCreateWindow(settings);
    ofRunApp(new ofApp());
}