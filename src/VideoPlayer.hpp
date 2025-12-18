#pragma once
#include <string>
#include <vector>
#include <iostream>

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

struct CachedFrame
{
    std::vector<uint8_t> rgbaFrameData;
    int width;
    int height;
    int64_t pts; // presentation timestamp

    CachedFrame(int w, int h, int64_t pts_, const std::vector<uint8_t>& data)
        : width(w), height(h), pts(pts_), rgbaFrameData(data)
    {
    }
};

class VideoPlayer
{
public:
    VideoPlayer();
    ~VideoPlayer();

    bool open(const std::string& path);
    void close();

    bool decodeOneFrame();
    bool seekSeconds(double t);
    void seek(double t);

    bool displayCachedFrame(double t);
    bool isTimeInsideCache(double t) const;

    void clearCache();
    void printCacheTimestamps() const;
    void trimCache(double maxAgeSeconds);
    std::vector<int> getAllKeyFramePts();

    int findCachedFrameIndexBySeconds(double t) const;
    bool ensureFrameCache(double secondsAhead);

    const uint8_t* rgbaData() const { return rgbaPlanes[0]; }
    int rgbaStride() const { return rgbaLinesize[0]; }
    int width() const { return videoWidth; }
    int height() const { return videoHeight; }
    GLuint texture() const { return videoTexture; }

    


    double fps() const { return videoFPS; }
    double durationSeconds() const { return videoDuration; }
    double currentTimeSeconds() const { return currentTime; }
    double currentCacheTimeSeconds() const { return currentCacheTime; }

    std::vector<CachedFrame> frameCache;

    int64_t secondsToPts(double t) const;
    double ptsToSeconds(int64_t pts) const;

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

    uint8_t* rgbaPlanes[4] = { nullptr, nullptr, nullptr, nullptr };
    int rgbaLinesize[4] = { 0, 0, 0, 0 };

    GLuint videoTexture = 0;

    double videoFPS = 30.0;
    double videoDuration = 0.0;
    double currentTime = 0.0;

    double currentCacheTime = 0.0;
    int currentCacheIndex = 0;

    bool convertFrameToRGBA();
    bool hasFrameWithPTS(int64_t pts) const;
    void insertFrameSorted(int64_t pts, uint8_t* rgba);
    
};
