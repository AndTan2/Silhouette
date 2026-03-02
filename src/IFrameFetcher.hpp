#pragma once
#include "DATA_TYPES.hpp"
#include <mutex>
#include <condition_variable>

class IFrameFetcher {
public:
    virtual ~IFrameFetcher() = default;
    virtual void pushFrame(const CachedFrame& frame) = 0;
    virtual void setVideoProperties(int width, int height, double fps, double duration, double timeBase,std::vector<int> Kframes) = 0;
    virtual void onSeekFinished(uint64_t seekToken) = 0;
    mutable std::mutex cacheMutex;
    std::condition_variable cv;

    std::atomic<uint64_t> finishedSeekToken{ 0 };
};

class IFrameCacheState {
public:
    virtual ~IFrameCacheState() = default;
    //virtual double cachedUntilSeconds() const = 0;

    virtual double getCurrentCacheTime() const = 0;
    //virtual double getTimeBase() const = 0;

    virtual double getCacheStartTime() const = 0;
   
    
    
};

class SeekControll {
public:
    virtual ~SeekControll() = default;

    virtual void newSeek(double t) = 0;
    virtual bool isSeekInProgress() const = 0;

    
};

