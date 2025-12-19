#pragma once
#include "DATA_TYPES.hpp"

class IFrameFetcher {
public:
    virtual ~IFrameFetcher() = default;
    virtual void pushFrame(const CachedFrame& frame) = 0;
    virtual void setVideoProperties(int width, int height, double fps, double duration, double timeBase,std::vector<int> Kframes) = 0;
};

class IFrameCacheState {
public:
    virtual ~IFrameCacheState() = default;
    //virtual double cachedUntilSeconds() const = 0;

    virtual double getCurrentCacheTime() const = 0;
    //virtual double getTimeBase() const = 0;
   
    
    
};

class SeekControll {
public:
    virtual ~SeekControll() = default;

    virtual void newSeek(double t) = 0;
    
};

