#pragma once
#include<string>
#include<iostream>
extern "C" {
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libswscale/swscale.h>
#include<libavutil/imgutils.h>
}

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#include<Windows.h>
#endif
#include<gl/GL.h>

class VideoPlayer
{
public:
	VideoPlayer();
	~VideoPlayer();

	bool open(const std::string& path);
	void close();

	bool decodeOneFrame();

	const uint8_t* rgbaData() const { return rgbaPlanes[0]; }
	int rgbaStride() const { return rgbaLinesize[0]; }
	int width() const { return videoWidth; }
	int height() const { return videoHeight; }
	GLuint texture() const { return videoTexture; }

	double fps() const { return videoFPS; }
	double durationSeconds() const { return videoDuration; }
	double currentTimeSeconds() const { return currentTime; }

	bool seekSeconds(double t);

	

private:
	bool opened = false;

	AVFormatContext* fmtCtx = nullptr;
	AVCodecContext* codecCtx = nullptr;
	AVStream* videoStream = nullptr;
	int videoStreamIndex = -1;

	AVFrame* frame = nullptr;
	AVPacket* packet = nullptr;

	SwsContext* swsCtx = nullptr;
	int videoWidth = 0;
	int videoHeight = 0;

	uint8_t* rgbaPlanes[4] = { nullptr,nullptr,nullptr,nullptr };
	int rgbaLinesize[4] = { 0,0,0,0 };

	GLuint videoTexture = 0;

	double videoFPS = 30.0;
	double videoDuration = 0.0; 
	double currentTime = 0.0;   

	bool convertFrameToRGBA();
};