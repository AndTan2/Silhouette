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



class VideoPlayer : public IFrameFetcher
    , public IFrameCacheState
{
public:
    VideoPlayer();
    ~VideoPlayer();


    
    void pushFrame(const CachedFrame& frame) override {
        insertFrameSorted(frame.pts, frame.rgbaFrameData);
    }

    double getCurrentCacheTime() const override {
        return currentCacheTime;
    }

    void setSeekController(SeekControll* seekController) {
        sc=seekController;
    }
   

    void setVideoProperties(int width, int height, double fps, double duration, double timeBase, std::vector<int> Kframes) override {
        videoWidth = width;
        videoHeight = height;
        videoFPS = fps;
        videoDuration = duration;
        videoTimeBase = timeBase;
        KeyFrames = Kframes;
    }
  
    void setDuration(double seconds) {
        videoDuration = seconds;
    }

    bool displayCachedFrame(double t);
    bool isTimeInsideCache(double t) const;

    void clearCache();
    void printCacheTimestamps() const;
    void trimCache(double maxAgeSeconds);

    void seek(double t);

    void playFromCache(double dt);

    int findCachedFrameIndexBySeconds(double t) const;
    

    const uint8_t* rgbaData() const { return rgbaPlanes[0]; }
    int rgbaStride() const { return rgbaLinesize[0]; }
    int width() const { return videoWidth; }
    int height() const { return videoHeight; }
    GLuint texture() const { return videoTexture; }

   

    double fps() const { return videoFPS; }
    double durationSeconds() const { return videoDuration; }
    

    double currentCacheTimeSeconds() const { return currentCacheTime; }

    std::vector<CachedFrame> frameCache;

    int64_t secondsToPts(double t) const;
    double ptsToSeconds(int64_t pts) const;

    void open();
    std::vector<int> KeyFrames;

private:
    
    SeekControll* sc = nullptr;

    int videoWidth = 0;
    int videoHeight = 0;

    uint8_t* rgbaPlanes[4] = { nullptr, nullptr, nullptr, nullptr };
    int rgbaLinesize[4] = { 0, 0, 0, 0 };

    GLuint videoTexture = 0;

    double videoFPS = 30.0;
    double videoDuration = 0.0;

    double currentCacheTime = 0.0;
    int currentCacheIndex = 0;
    double videoTimeBase = 0;

 
    
    

    bool hasFrameWithPTS(int64_t pts) const;
    void insertFrameSorted(int64_t pts, std::vector<uint8_t> rgbaFrameData);
    
};
