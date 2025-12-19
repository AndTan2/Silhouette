#pragma once

#include "Input.hpp"
#include "Camera.hpp"
#include "VideoPlayer.hpp"
#include "stb_image.h"
#include "Decoder.hpp"

#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>

class App
{
public:
    bool init();
    void run();
    void shutdown();

    void drawScrubber();
    void onScroll(double xoffset, double yoffset);

private:
    GLFWwindow* window = nullptr;


    GLuint imageTexture = 0;
    int imageWidth = 0;
    int imageHeight = 0;

    Input input;
    Camera camera;
    VideoPlayer vp;
    Decoder dec;


    GLFWcursor* openHandCursor = nullptr;
    GLFWcursor* closedHandCursor = nullptr;


    std::chrono::time_point<std::chrono::high_resolution_clock> frameEnd;
    double frameDuration = 0.0;
    double lastTime = 0.0;


    bool playState = false;


    int width = 1280;
    int height = 720;


    float scrubberY0 = 0.0f;
    float scrubberY1 = 0.0f;


    std::vector<int> keyframePts;


    bool loadTestImage(const char* path);
};
