#pragma once

struct CachedFrame
{
    std::vector<uint8_t> rgbaFrameData;
    int width;
    int height;
    int64_t pts; 

    CachedFrame(int w, int h, int64_t pts_, const std::vector<uint8_t>& data)
        : width(w), height(h), pts(pts_), rgbaFrameData(data)
    {
    }
};