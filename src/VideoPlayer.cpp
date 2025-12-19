#include "VideoPlayer.hpp"
#include <algorithm>
#include <cassert>

VideoPlayer::VideoPlayer() 
{

    

}

VideoPlayer::~VideoPlayer() {
    //close();
}


void VideoPlayer::open()
{
    glGenTextures(1, &videoTexture);
    glBindTexture(GL_TEXTURE_2D, videoTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight,
        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );
    glBindTexture(GL_TEXTURE_2D, 0);
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
        << numFrames * videoHeight * videoWidth / (256 * 1024) << " Mb)\n";
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
    double cutoff = currentCacheTime - maxAgeSeconds;
    double cutoffFuture = currentCacheTime + 7;

    // trim old frames
    while (!frameCache.empty()) {
        double frameTime = ptsToSeconds(frameCache.front().pts);
        if (frameTime < cutoff) {
            frameCache.erase(frameCache.begin());
        }
        else {
            break;
        }
    }

    // trim frames too far ahead
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

    if (targetPts < frameCache.front().pts || targetPts > frameCache.back().pts) return -1;

    auto it = std::upper_bound(
        frameCache.begin(), frameCache.end(), targetPts,
        [](int64_t value, const CachedFrame& f) { return value < f.pts; }
    );
    --it;

    return static_cast<int>(std::distance(frameCache.begin(), it));
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
    int idx = findCachedFrameIndexBySeconds(t);
    if (idx < 0 || idx >= frameCache.size())
        return false;

 
    if (idx == currentCacheIndex)
        return true;

    const CachedFrame& cf = frameCache[idx];

    glBindTexture(GL_TEXTURE_2D, videoTexture);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0, cf.width, cf.height,
        GL_RGBA, GL_UNSIGNED_BYTE, cf.rgbaFrameData.data()
    );
    glBindTexture(GL_TEXTURE_2D, 0);

    currentCacheTime = ptsToSeconds(cf.pts);
    currentCacheIndex = idx;

    std::cout << idx << " new frame displayed\n";
    return true;
}


bool VideoPlayer::isTimeInsideCache(double t) const
{
    if (frameCache.empty()) return false;

    int64_t targetPts = secondsToPts(t);

    for (size_t i = 0; i < frameCache.size(); ++i) {
        int64_t frameStart = frameCache[i].pts;
        int64_t frameEnd = frameStart + secondsToPts(1.0 / fps());
        if (targetPts >= frameStart && targetPts < frameEnd) {
            //std::cout << "CACHE FOUND!!!\n";
            return true;
        }
    }

    std::cout << "click not inside cache\n";
    return false;
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

void VideoPlayer::insertFrameSorted(int64_t pts, std::vector<uint8_t> rgbaFrameData)
{
    auto it = std::lower_bound(
        frameCache.begin(), frameCache.end(), pts,
        [](const CachedFrame& f, int64_t p) { return f.pts < p; }
    );

    if (it != frameCache.end() && it->pts == pts) {
        return;
    }

    frameCache.insert(it, CachedFrame(videoWidth, videoHeight, pts, std::move(rgbaFrameData)));
}

void VideoPlayer::seek(double t)
{
    if (!isTimeInsideCache(t)) {

        sc->newSeek(t);

    }
   
    displayCachedFrame(t);
    trimCache(2);
    currentCacheTime = t;
    
}

void VideoPlayer::playFromCache(double dt)
{
    if (frameCache.empty()) return; // nothing to display

    // Advance playhead
    currentCacheTime += dt;

    // Looping at the end
    if (currentCacheTime > videoDuration) {
        currentCacheTime = 0.0;
    }

    // Display the frame at the current cache time
    displayCachedFrame(currentCacheTime);
}