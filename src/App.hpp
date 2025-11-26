#pragma once
#include "Input.hpp"

#include"stb_image.h"

#include <iostream>
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#include<Windows.h>
#endif
#include<gl/GL.h>
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

	float zoom = 1.0f;
	float panX = 0.0f;
	float panY = 0.0f;
	float interpolatedZoom = 1.0f;
	float interpolatedPanX = 0.0f;
	float interpolatedPanY = 0.0f;
	bool panning = false;

	Input input;

	GLFWcursor* handCursor;

	std::chrono::time_point<std::chrono::high_resolution_clock> frameEnd;
	double frameDuration = 0;
	double lastTime;

	GLFWcursor* openHandCursor = nullptr;



	bool loadTestImage(const char* path);

};