#pragma once

#include <glad/glad.h>
#include "VideoPlayer.hpp"
#include "shaderClass.hpp"
#include <vector>


class Scrubber {
public:
    Scrubber();
    ~Scrubber();
    void init(VideoPlayer* videoPlayer);
    void draw(int windowWidth, int windowHeight);
    void zoom(float scrollAmount, float mouseX, float windowWidth);
    void addPanDelta(float dx);
    void update(float dt);

    float interpolatedOffset = 0.0f;
    float zoomFactor = 1.0f;

    float scrubberMarkerY = 0.0f;    // top of the scrubber bar
    float scrubberBarHeight = 0.0f;  // its current height

private:
    VideoPlayer* vp = nullptr;
    float scrubberY0 = 0.0f;
    float minZoom = 1.0f;
    float maxZoom = 5000.0f;
    float offset = 0.0f;

    bool panning = false;

    // Modern OpenGL
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    Shader* shader = nullptr;
    GLint colorUniformLoc = -1;
    GLint transformUniformLoc = -1;
    int currentWindowWidth;
    int currentWindowHeight;

    int numFramesAtMaxZoom = 5; 

    void createGeometry();
    void renderQuad(float x, float y, float width, float height, float r, float g, float b, int windowWidth, int windowHeight);
    float screenXToTimeline(float screenX, int windowWidth);
    float timelineToX(float t);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};