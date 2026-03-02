#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "DATA_TYPES.hpp"
#include "IFrameFetcher.hpp"
#include <chrono>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

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
		{
			seekTarget.store(t, std::memory_order_relaxed);
			seekToken.fetch_add(1, std::memory_order_release);
		}
		seekCV.notify_one();
	}

	bool isSeekInProgress() const override {
		return seekInProgress.load(std::memory_order_acquire);
	}

	std::vector<int> getAllKeyFramePts();
	std::vector<int> keyframePts;

	bool decodeOneFrame();
	bool seekSeconds(double t);

	int width() const { return videoWidth; }
	int height() const { return videoHeight; }

	double fps() const { return videoFPS; }

	void sendInfoToPlayer();

	double durationSeconds() const { return videoDuration; }

	double currentTimeSeconds() const { return currentDecoderTime; }

	void ensureSufficintFrames();

	void decodingLoop();

	void startDecodingThread();

	void stopDecodingThread();

	bool decodeLastSegment(double target);

	void benchmark();

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
	double currentDecoderTime = 0.0;

	bool convertFrameToRGBA();
	bool ensureCacheAhead(double secondsAhead);
	bool ensureCacheBehind(double tolerance);
	std::vector<uint8_t> copyRGBA(uint8_t* src);

	std::thread decodeThread;
	std::atomic<bool> running{ false };
	std::mutex decodeMutex;
	std::condition_variable decodeCV;

	std::condition_variable seekCV;

	std::atomic<uint64_t> seekToken{ 0 };
	std::atomic<double>  seekTarget{ 0.0 };
	std::atomic<bool> seekInProgress{ false };
	std::atomic<uint64_t> seekCompletedToken{ 0 };




};