#pragma once
#include "Input.hpp"
#include "Camera.hpp"
#include "VideoPlayer.hpp"

#include"stb_image.h"

#include <chrono>
#include <thread>

struct GLFWwindow;

class App
{

public:

	bool init();

	void run();

	void shutdown();


	void onScroll(double xoffset, double yoffset);

private:
	GLFWwindow* window = nullptr;

	unsigned int imageTexture = 0;
	int imageWidth = 0;
	int imageHeight = 0;

	Input input;
	Camera camera;
	VideoPlayer vp;

	GLFWcursor* handCursor;

	std::chrono::time_point<std::chrono::high_resolution_clock> frameEnd;
	double frameDuration = 0;
	double lastTime;
	bool playState = false;

	GLFWcursor* openHandCursor = nullptr;
	GLFWcursor* closedHandCursor = nullptr;


	bool loadTestImage(const char* path);

};