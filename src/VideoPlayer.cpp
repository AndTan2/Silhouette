#include "VideoPlayer.hpp"
#include <algorithm>
#include <cassert>

VideoPlayer::VideoPlayer() 
{

    

}

VideoPlayer::~VideoPlayer() {
 
}


void VideoPlayer::open()
{
    if (!yuvRenderer.init(videoWidth, videoHeight)) {
        std::cerr << "Failed to initialize YUVRenderer\n";
        return;
    }
    rendererInitialized = true;
    std::cout << "Video texture size: " << videoWidth << "x" << videoHeight << "\n";


}

bool VideoPlayer::hasFrameWithPTS(int64_t pts) const
{
    auto it = std::lower_bound(
        frameCache.begin(), frameCache.end(), pts,
        [](const CachedFrame& f, int64_t value) { return f.pts < value; }
    );
    return it != frameCache.end() && it->pts == pts;
}

void VideoPlayer::clearCache()
{
    int numFrames = frameCache.size();
    frameCache.clear();
    frameCache.shrink_to_fit();

    std::cout << "Cache cleared\n";
    std::cout << "cleared " << numFrames << " frames from cache ("
        << numFrames * videoHeight * videoWidth * 1.5f / (1024 * 1024) << " Mb)\n";
}

void VideoPlayer::printCacheTimestamps() const
{
    if (frameCache.empty()) {
        std::cout << "Cache is empty.\n";
        return;
    }


    std::cout << "Frame timestamps in seconds:\n";
    for (size_t i = 0; i < frameCache.size(); ++i) {
        const auto& cf = frameCache[i];
        double seconds = cf.pts * videoTimeBase;
        std::cout << "Frame " << i + 1 << ": " << seconds << " s\n";
    }
}

void VideoPlayer::trimCache(double maxAgeSeconds)
{
     std::lock_guard<std::mutex> lock(cacheMutex);
    double cutoff = currentCacheTime - maxAgeSeconds;
    double cutoffFuture = currentCacheTime + 7;

 
    while (!frameCache.empty()) {
        double frameTime = ptsToSeconds(frameCache.front().pts);
        if (frameTime < cutoff) {
            frameCache.erase(frameCache.begin());
        }
        else {
            break;
        }
    }

    
    while (!frameCache.empty()) {
        double frameTime = ptsToSeconds(frameCache.back().pts);
        if (frameTime > cutoffFuture) {
            frameCache.pop_back();
        }
        else {
            break;
        }
    }
}



int VideoPlayer::findCachedFrameIndexBySeconds(double t) const
{
    if (frameCache.empty()) return -1;

    int64_t targetPts = secondsToPts(t);

    // Binary search for lower bound
    auto it = std::lower_bound(
        frameCache.begin(), frameCache.end(), targetPts,
        [](const CachedFrame& f, int64_t value) { return f.pts < value; }
    );

    // Handle edge cases
    if (it == frameCache.begin()) return 0;
    if (it == frameCache.end()) return static_cast<int>(frameCache.size() - 1);

    // Compare with previous frame to find closest
    auto prev = it - 1;
    if (std::abs(prev->pts - targetPts) <= std::abs(it->pts - targetPts)) {
        return static_cast<int>(std::distance(frameCache.begin(), prev));
    }
    else {
        return static_cast<int>(std::distance(frameCache.begin(), it));
    }
}


int64_t VideoPlayer::secondsToPts(double t) const
{
    return static_cast<int64_t>(t / videoTimeBase);
}

double VideoPlayer::ptsToSeconds(int64_t pts) const
{
    return pts * videoTimeBase;
}


bool VideoPlayer::displayCachedFrame(double t)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    int idx = findCachedFrameIndexBySeconds(t);
    if (idx < 0 || idx >= frameCache.size())
        return false;

 
    if (idx == currentCacheIndex)
        return true;

    const CachedFrame& cf = frameCache[idx];

    yuvRenderer.uploadFrame(cf.yData(), cf.uData(), cf.vData(),
        cf.yStride(), cf.uStride(),
        cf.width, cf.height);

    currentCacheTime = ptsToSeconds(cf.pts);
    currentCacheIndex = idx;

    std::cout << idx << " new frame displayed\n";
    return true;
}

void VideoPlayer::render(float x0, float x1, float y0, float y1, int screenWidth, int screenHeight)
{
    yuvRenderer.renderRect(x0, x1, y0, y1, screenWidth, screenHeight);
}

bool VideoPlayer::isTimeInsideCache(double t) const
{
    if (frameCache.empty()) return false;

    int64_t targetPts = secondsToPts(t);

    // Find nearest frame
    const CachedFrame* nearest = nullptr;
    int64_t minDiff = std::numeric_limits<int64_t>::max();

    for (const auto& frame : frameCache) {
        int64_t diff = std::abs(frame.pts - targetPts);
        if (diff < minDiff) {
            minDiff = diff;
            nearest = &frame;
        }
    }

    // Check if within reasonable range (e.g., half a frame duration)
    int64_t halfFrameDuration = secondsToPts(0.8 / fps());
    bool found = (minDiff <= halfFrameDuration);

    if (!found) {
        std::cout << "Time " << t << " not inside cache (diff: "
            << minDiff << ", max allowed: " << halfFrameDuration << ")\n";
    }

    return found;
}


//int64_t VideoPlayer::secondsToPts(double t) const
//{
//    
//    double tb = getTimeBase();
//    return static_cast<int64_t>(t / tb);
//}
//
//double VideoPlayer::ptsToSeconds(int64_t pts) const
//{
//    return pts * getTimeBase();
//}

void VideoPlayer::insertFrameSorted(CachedFrame frame)
{
    int64_t pts = frame.pts;
    auto it = std::lower_bound(
        frameCache.begin(), frameCache.end(), pts,
        [](const CachedFrame& f, int64_t p) { return f.pts < p; }
    );

    if (it != frameCache.end() && it->pts == pts) return; // duplicate

    frameCache.insert(it, std::move(frame));
}



void VideoPlayer::seek(double t)
{
    bool flag = isTimeInsideCache(t);
    
    if (!flag) 
    {
        sc->newSeek(t);
    }
    
    currentCacheTime = t;

    if (flag)
    {
        displayCachedFrame(t);
        trimCache(6);
    }
}

void VideoPlayer::playFromCache(double dt)
{
    if (frameCache.empty()) return; 


    currentCacheTime += dt;

 
    if (currentCacheTime > videoDuration) {
        currentCacheTime = 0.0;
    }

    
    displayCachedFrame(currentCacheTime);
}

void VideoPlayer::update()
{
    static uint64_t lastHandled = 0;

    uint64_t token =
        finishedSeekToken.load(std::memory_order_acquire);

    if (token != lastHandled) {
        lastHandled = token;

        
        onSeekCompletedMainThread();
    }
}

void VideoPlayer::onSeekCompletedMainThread()
{

    for (int i = 0; i < 5; i++)
        std::cout << "                                    SEEK COMPLETE SEEK COMPLETE SEEK COMPLETE SEEK COMPLETE \n";
    displayCachedFrame(currentCacheTime);
    trimCache(6);

}
