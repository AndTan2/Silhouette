#pragma once
#include "VideoPlayer.hpp"




class Scrubber
{
public:

	Scrubber();



	void init(VideoPlayer* videoPlayer);

	void zoom(float scrollAmount, float mouseX, float windowWidth);

	void draw();

	void addPanDelta(float dx);

	void update(float dt);


	float zoomFactor = 1.0f;
	float offset = 0.0f;
	float interpolatedOffset = 0.0f;
private:

	VideoPlayer* vp;

	float scrubberY0 = 0.0f;
	float scrubberY1 = 0.0f;


	const float minZoom = 1.0f;
	const float maxZoom = 10.0f;


	bool panning = false;



};