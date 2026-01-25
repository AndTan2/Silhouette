#pragma once

#include "Input.hpp"
#include "Camera.hpp"
#include "VideoPlayer.hpp"
#include "Decoder.hpp"
#include "Scrubber.hpp"
#include "YUVRenderer.hpp"

#include <chrono>
#include <thread>

class App
{
public:
    bool init();
    void run();
    void shutdown();

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
    Scrubber scrb;
    YUVRenderer yuvRenderer;

    bool useYUVRenderer = false;

    GLFWcursor* openHandCursor = nullptr;
    GLFWcursor* closedHandCursor = nullptr;


    std::chrono::time_point<std::chrono::high_resolution_clock> frameEnd;
    double frameDuration = 0.0;
    double lastTime = 0.0;


    bool playState = false;


    int width = 1280;
    int height = 720;

    std::vector<int> keyframePts;

};
