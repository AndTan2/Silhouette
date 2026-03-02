#pragma once
#include <cstdint>

extern "C" {
#include <libavutil/frame.h>
}

struct CachedFrame {
    int width = 0;
    int height = 0;
    int64_t pts = 0;
    AVFrame* frame = nullptr;

    // Constructor - zero copy, just refs the frame
    CachedFrame(int w, int h, int64_t pts, AVFrame* src)
        : width(w), height(h), pts(pts)
    {
        frame = av_frame_alloc();
        av_frame_ref(frame, src);
    }

    // Default constructor
    CachedFrame() = default;

    // Destructor - releases the ref, FFmpeg reclaims buffer
    ~CachedFrame() {
        if (frame) {
            av_frame_unref(frame);
            av_frame_free(&frame);
        }
    }

    // No copying - would mess up refcounts
    CachedFrame(const CachedFrame&) = delete;
    CachedFrame& operator=(const CachedFrame&) = delete;

    // Moving is fine
    CachedFrame(CachedFrame&& o) noexcept
        : width(o.width), height(o.height), pts(o.pts), frame(o.frame)
    {
        o.frame = nullptr;
    }
    CachedFrame& operator=(CachedFrame&& o) noexcept {
        if (this != &o) {
            if (frame) { av_frame_unref(frame); av_frame_free(&frame); }
            width = o.width; height = o.height; pts = o.pts; frame = o.frame;
            o.frame = nullptr;
        }
        return *this;
    }

    // Access planes the same way you did before
    uint8_t* yData() const { return frame ? frame->data[0] : nullptr; }
    uint8_t* uData() const { return frame ? frame->data[1] : nullptr; }
    uint8_t* vData() const { return frame ? frame->data[2] : nullptr; }

    int yStride() const { return frame ? frame->linesize[0] : 0; }
    int uStride() const { return frame ? frame->linesize[1] : 0; }
    int vStride() const { return frame ? frame->linesize[2] : 0; }

    bool isValid() const { return frame != nullptr && width > 0 && height > 0; }
};