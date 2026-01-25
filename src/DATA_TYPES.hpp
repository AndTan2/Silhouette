#pragma once

struct CachedFrame {
    int width = 0;
    int height = 0;
    int64_t pts = 0;
    std::vector<uint8_t> rgbaFrameData;  


    CachedFrame(int w, int h, int64_t pts, std::vector<uint8_t>&& data)
        : width(w), height(h), pts(pts), rgbaFrameData(std::move(data)) {
    }

    // Default constructor
    CachedFrame() = default;
};