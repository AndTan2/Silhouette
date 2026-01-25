#pragma once
#include <iostream>
#define GLFW_INCLUDE_NONE
#include<glad/glad.h>
#include <GLFW/glfw3.h>


class Camera {
public:
	void beginFrame(double dt);
	void setPanning(bool enabled) { _panning = enabled; }
	void addPanDelta(float dx, float dy);
	void onScroll(float scrollY, float mouseX, float mouseY, int screenWidth, int screenHeight);
	void computeImageRect(int screenWidth, int screenHeight, int imageWidth, int imageHeight, float& x0, float& x1, float& y0, float& y1) const;

private:
	float _zoom = 1.0f;
	float _panX = 0.0f;
	float _panY = 0.0f;
	float _interpolatedZoom = 1.0f;
	float _interpolatedPanX = 0.0f;
	float _interpolatedPanY = 0.0f;
	bool _panning = false;

	float _minZoom = 0.1f;
	float _maxZoom = 20.0f;

	float _smoothK = 50.0f;
	float _zoomFullView = 2.0f;
	float _maxRecenterStrength = 0.20f;
	float _recenterCurveK = 2.0f;
};