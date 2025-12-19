#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "DATA_TYPES.hpp"
#include "IFrameFetcher.hpp"


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#include <Windows.h>
#endif
#include <gl/GL.h>

class Decoder : public SeekControll
{
public:
	Decoder();
	~Decoder();

	bool open(const std::string& path);
	void close();

	void setFrameFetcher(IFrameFetcher* fetcher) {
		frameFetcher = fetcher;
	}

	void setCacheState(const IFrameCacheState* state) {
		cacheState = state;
	}

	void newSeek(double t) override {
		seekSeconds(t);
	}

	std::vector<int> getAllKeyFramePts();

	bool decodeOneFrame();
	bool seekSeconds(double t);

	int width() const { return videoWidth; }
	int height() const { return videoHeight; }

	double fps() const { return videoFPS; }

	void sendInfoToPlayer();

	double durationSeconds() const { return videoDuration; }

	double currentTimeSeconds() const { return currentTime; }

	void ensureSufficintFrames();

private:
	bool opened = false;

	AVFormatContext* fmtCtx = nullptr;
	AVCodecContext* codecCtx = nullptr;
	AVStream* videoStream = nullptr;
	int videoStreamIndex = -1;

	AVFrame* frame = nullptr;
	AVPacket* packet = nullptr;

	SwsContext* swsCtx = nullptr;

	IFrameFetcher* frameFetcher = nullptr;
	const IFrameCacheState* cacheState = nullptr;

	int videoWidth = 0;
	int videoHeight = 0;

	uint8_t* rgbaPlanes[4] = { nullptr, nullptr, nullptr, nullptr };
	int rgbaLinesize[4] = { 0, 0, 0, 0 };

	double videoFPS = 30.0;
	double videoDuration = 0.0;
	double currentTime = 0.0;

	bool convertFrameToRGBA();
	bool ensureFrameCache(double secondsAhead);
	std::vector<uint8_t> copyRGBA(uint8_t* src);



};