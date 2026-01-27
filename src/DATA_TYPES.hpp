#pragma once

//struct CachedFrame {
//    int width = 0;
//    int height = 0;
//    int64_t pts = 0;
//    std::vector<uint8_t> rgbaFrameData;  
//
//
//    CachedFrame(int w, int h, int64_t pts, std::vector<uint8_t>&& data)
//        : width(w), height(h), pts(pts), rgbaFrameData(std::move(data)) {
//    }
//
//    // Default constructor
//    CachedFrame() = default;
//};

struct CachedFrame {
    int width = 0;
    int height = 0;
    int64_t pts = 0;

    // YUV planes (separate vectors for each plane)
    std::vector<uint8_t> yPlane;
    std::vector<uint8_t> uPlane;
    std::vector<uint8_t> vPlane;

    // Constructor for YUV data
    CachedFrame(int w, int h, int64_t pts,
        std::vector<uint8_t>&& yData,
        std::vector<uint8_t>&& uData,
        std::vector<uint8_t>&& vData)
        : width(w), height(h), pts(pts),
        yPlane(std::move(yData)),
        uPlane(std::move(uData)),
        vPlane(std::move(vData)) {
    }

    // Default constructor
    CachedFrame() = default;

    // Optional: Helper method to calculate expected sizes
    size_t getYSize() const { return width * height; }
    size_t getUSize() const { return (width / 2) * (height / 2); }
    size_t getVSize() const { return (width / 2) * (height / 2); }

    // Optional: Check if frame has valid YUV data
    bool isValidYUV() const {
        return width > 0 && height > 0 &&
            yPlane.size() >= getYSize() &&
            uPlane.size() >= getUSize() &&
            vPlane.size() >= getVSize();
    }
};