#include "Decoder.hpp"
#include <cassert>


Decoder::Decoder()
{

    
    
}

Decoder::~Decoder()
{
    close();
}

bool Decoder::open(const std::string& path)
{
    close();

    if (avformat_open_input(&fmtCtx, path.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Failed to open video: " << path << "\n";
        return false;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        std::cerr << "Failed to read stream info.\n";
        close();
        return false;
    }

    videoStreamIndex = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
        AVStream* s = fmtCtx->streams[i];
        if (s->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = static_cast<int>(i);
            videoStream = s;
            break;
        }
    }

    if (videoStreamIndex < 0) {
        std::cerr << "No video found in file.\n";
        close();
        return false;
    }

    AVCodecParameters* codecpar = videoStream->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        std::cerr << "Failed to find decoder.\n";
        close();
        return false;
    }

    double streamFps = av_q2d(videoStream->avg_frame_rate);
    if (streamFps < 1.0f) {
        streamFps = av_q2d(videoStream->r_frame_rate);
    }
    if (streamFps < 1.0) {
        streamFps = 25.0; // safe default
    }
    videoFPS = streamFps;

    // compute duration in seconds
    if (fmtCtx->duration != AV_NOPTS_VALUE) {
        videoDuration = fmtCtx->duration / (double)AV_TIME_BASE;
    }
    else if (videoStream->duration != AV_NOPTS_VALUE) {
        videoDuration = videoStream->duration * av_q2d(videoStream->time_base);
    }
    else {
        videoDuration = 0.0;
    }

    currentTime = 0.0;

    std::cout << "Video FPS: " << videoFPS << "\n";
    std::cout << "Video duration: " << videoDuration << " sec\n";

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        std::cerr << "Failed to allocate codec context.\n";
        close();
        return false;
    }

    if (avcodec_parameters_to_context(codecCtx, codecpar) < 0) {
        std::cerr << "Failed to copy codec parameters to context.\n";
        close();
        return false;
    }

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        std::cerr << "Failed to open codec.\n";
        close();
        return false;
    }

    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet) {
        std::cerr << "Failed to alocate frame or packet.\n";
        close();
        return false;
    }

    videoWidth = codecCtx->width;
    videoHeight = codecCtx->height;

    swsCtx = sws_getContext(
        videoWidth, videoHeight, codecCtx->pix_fmt,
        videoWidth, videoHeight, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!swsCtx) {
        std::cerr << "Failed to create SwsContext.\n";
        close();
        return false;
    }

    int ret = av_image_alloc(rgbaPlanes, rgbaLinesize, videoWidth, videoHeight, AV_PIX_FMT_RGBA, 1);
    if (ret < 0) {
        std::cerr << "Failed to alocate RGBA image buffer.\n";
        close();
        return false;
    }

    

    opened = true;
    std::cout << "Opened video: " << path << "\n";
    std::cout << " resolution:" << codecCtx->width << "x" << codecCtx->height << "\n";

    return true;
}

void Decoder::close()
{
    if (rgbaPlanes[0]) {
        av_freep(&rgbaPlanes[0]);
    }
    for (int i = 0; i < 4; ++i) {
        rgbaPlanes[i] = nullptr;
        rgbaLinesize[i] = 0;
    }

    if (swsCtx) {
        sws_freeContext(swsCtx);
        swsCtx = nullptr;
    }

    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if (codecCtx) {
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
    }

    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
        fmtCtx = nullptr;
    }


    videoStream = nullptr;
    videoStreamIndex = -1;
    opened = false;
}

bool Decoder::convertFrameToRGBA()
{
    if (!swsCtx || !frame || !rgbaPlanes[0]) return false;

    int rows = sws_scale(
        swsCtx,
        frame->data,
        frame->linesize,
        0,
        frame->height,   
        rgbaPlanes,
        rgbaLinesize
    );

    if (rows != frame->height) {
        std::cerr << "Warning: sws_scale wrote " << rows << " rows instead of " << frame->height << "\n";
    }

    

    return true;
}

bool Decoder::decodeOneFrame()
{
    if (!opened) {
        std::cerr << "decodeOneFrame called but video not opened.\n";
        return false;
    }

    while (true) {
        int ret = av_read_frame(fmtCtx, packet);
        if (ret < 0) {
            std::cerr << "End of file or error reading frame.\n";
            return false;
        }

        if (packet->stream_index != videoStreamIndex) {
            av_packet_unref(packet);
            continue;
        }

        ret = avcodec_send_packet(codecCtx, packet);
        av_packet_unref(packet);
        if (ret < 0) {
            std::cerr << "Error sending packet to decoder.\n";
            return false;
        }

        while (true) {
            ret = avcodec_receive_frame(codecCtx, frame);
            if (ret == AVERROR(EAGAIN)) {
                break;
            }
            else if (ret == AVERROR_EOF) {
                std::cerr << "Decoder signaled EOF.\n";
                return false;
            }
            else if (ret < 0) {
                std::cerr << "Error decoding frame.\n";
                return false;
            }

            int64_t pts = frame->best_effort_timestamp;
            if (pts == AV_NOPTS_VALUE) pts = frame->pts;

            if (pts != AV_NOPTS_VALUE) {
                double seconds = pts * av_q2d(videoStream->time_base);
                currentTime = seconds;
                //std::cout << " pts: " << pts << " (" << seconds << " sec)\n";
            }
            else {
                std::cout << " pts: N/A\n";
            }

            if (!convertFrameToRGBA()) {
                std::cerr << "Failed to convert frame to RGBA.\n";
                return false;
            }

            int64_t pts2 = frame->best_effort_timestamp;
            if (pts2 == AV_NOPTS_VALUE) pts2 = frame->pts;
            if (pts2 != AV_NOPTS_VALUE) {
                std::vector<uint8_t> rgbaCopy = copyRGBA(rgbaPlanes[0]);

                CachedFrame frame(
                    videoWidth,
                    videoHeight,
                    pts2,
                    rgbaCopy
                );


                if (frameFetcher) {
                    frameFetcher->pushFrame(frame);
                }


                return true;
            }
        }
    }
}



bool Decoder::ensureFrameCache(double secondsAhead)
{
    const double target = cacheState->getCurrentCacheTime() + secondsAhead;
    while (currentTime < target) {
        if (!decodeOneFrame()) return false;
    }
    return true;
}

bool Decoder::seekSeconds(double t)
{
    if (!opened || !videoStream) return false;

    if (videoDuration > 0.0) {
        if (t < 0.0) t = 0.0;
        if (t > videoDuration) t = videoDuration;
    }

    double tb = av_q2d(videoStream->time_base);
    int64_t targetPts = static_cast<int64_t>(t / tb);

    std::cout << "SeekSeconds requested: t=" << t << " sec, targetPts=" << targetPts << "\n";

    int ret = av_seek_frame(fmtCtx, videoStreamIndex, targetPts, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        std::cerr << "Failed to seek to " << t << " sec\n";
        return false;
    }

    avcodec_flush_buffers(codecCtx);

    const double epsilon = 0.5 * (1.0 / videoFPS);
    while (true) {
        if (!decodeOneFrame()) {
            std::cerr << "Reached EOF while seeking.\n";
            return false;
        }

        double ct = currentTimeSeconds();
        if (ct + epsilon >= t) {
            std::cout << "Seeked, now at ~" << ct << " sec\n";
            break;
        }
    }

    return true;
}

void Decoder::sendInfoToPlayer()
{
    decodeOneFrame();
    auto keyFrames = getAllKeyFramePts();
    frameFetcher->setVideoProperties(videoWidth, videoHeight, videoFPS, videoDuration, av_q2d(videoStream->time_base), keyFrames);
    
    
}

std::vector<int> Decoder::getAllKeyFramePts()
{
    std::vector<int> keyframePts;
    AVPacket* pkt = av_packet_alloc();

    av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);

    while (av_read_frame(fmtCtx, pkt) >= 0) {
        if (pkt->stream_index == videoStreamIndex && (pkt->flags & AV_PKT_FLAG_KEY)) {
            keyframePts.push_back(pkt->pts);
        }
        av_packet_unref(pkt);
    }

    av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
    av_packet_free(&pkt);

    return keyframePts;
}


//void Decoder::seek(double t)
//{
//    if (!isTimeInsideCache(t)) {
//        seekSeconds(t);
//    }
//
//    currentCacheTime = t;
//    ensureFrameCache(2);
//    displayCachedFrame(t);
//    trimCache(2);
//}

std::vector<uint8_t> Decoder::copyRGBA(uint8_t* src)
{
    size_t size = videoWidth * videoHeight * 4;
    std::vector<uint8_t> dst(size);
    std::memcpy(dst.data(), src, size);
    return dst;
}

void Decoder::ensureSufficintFrames()
{
    
    ensureFrameCache(0.5);


}

