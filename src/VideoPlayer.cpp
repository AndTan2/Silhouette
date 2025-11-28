#include "VideoPlayer.hpp"

VideoPlayer::VideoPlayer()
{

}

VideoPlayer::~VideoPlayer()
{
	close();
}

bool VideoPlayer::open(const std::string& path)
{
	close();

	if (avformat_open_input(&fmtCtx, path.c_str(), nullptr, nullptr) < 0)
	{
		std::cerr << "Failed to open video: " << path << "\n";
		return false;
	}

	if (avformat_find_stream_info(fmtCtx, nullptr) < 0)
	{
		std::cerr << "Failed to read stream info.\n";
		close();
		return false;
	}

	videoStreamIndex = -1;
	for (unsigned i = 0; i < fmtCtx->nb_streams; ++i)
	{
		AVStream* s = fmtCtx->streams[i];
		if (s->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStreamIndex = static_cast<int>(i);
			videoStream = s;
			break;
		}
	}

	if (videoStreamIndex < 0)
	{
		std::cerr << "No video found in file.\n";
		close();
		return false;
	}

	AVCodecParameters* codecpar = videoStream->codecpar;
	const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
	if (!codec)
	{
		std::cerr << "Failed to find decoder.\n";
		close();
		return false;
	}

	double streamFps = av_q2d(videoStream->avg_frame_rate);
	if (streamFps < 1.0f)
	{
		streamFps = av_q2d(videoStream->r_frame_rate);
	}

	if (streamFps < 1.0)
	{
		streamFps = 25.0; // safe default
	}
	videoFPS = streamFps;

	// compute duration in seconds
	if (fmtCtx->duration != AV_NOPTS_VALUE)
	{
		// duration is in AV_TIME_BASE units (microseconds-ish)
		videoDuration = fmtCtx->duration / (double)AV_TIME_BASE;
	}
	else if (videoStream->duration != AV_NOPTS_VALUE)
	{
		videoDuration = videoStream->duration * av_q2d(videoStream->time_base);
	}
	else
	{
		videoDuration = 0.0; // unknown
	}

	currentTime = 0.0;

	std::cout << "Video FPS: " << videoFPS << "\n";
	std::cout << "Video duration: " << videoDuration << " sec\n";

	codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx)
	{
		std::cerr << "Failed to allocate codec context.\n";
		close();
		return false;
	}

	if (avcodec_parameters_to_context(codecCtx, codecpar) < 0)
	{
		std::cerr << "Failed to copy codec parameters to context.\n";
		close();
		return false;
	}

	if (avcodec_open2(codecCtx, codec, nullptr) < 0)
	{
		std::cerr << "Failed to open codec.\n";
		close();
		return false;
	}

	frame = av_frame_alloc();
	packet = av_packet_alloc();

	if (!frame || !packet)
	{
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

	if (!swsCtx)
	{
		std::cerr << "Failed to create SwsContext.\n";
		close();
		return false;
	}

	int ret = av_image_alloc(
		rgbaPlanes,
		rgbaLinesize,
		videoWidth,
		videoHeight,
		AV_PIX_FMT_RGBA,
		1
	);

	if (ret < 0)
	{
		std::cerr << "Failed to alocate RGBA image buffer.\n";
		close();
		return false;
	}

	glGenTextures(1, &videoTexture);
	glBindTexture(GL_TEXTURE_2D, videoTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(
		GL_TEXTURE_2D, 0,
		GL_RGBA,
		videoWidth, videoHeight, 0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	opened = true;

	std::cout << "Opened video: " << path << "\n";
	std::cout << " resolution:" << codecCtx->width << "x" << codecCtx->height << "\n";

	return true;
}

void VideoPlayer::close()
{
	if (rgbaPlanes[0])
	{
		av_freep(&rgbaPlanes[0]);
	}
	for (int i = 0; i < 4; ++i)
	{
		rgbaPlanes[i] = nullptr;
		rgbaLinesize[i] = 0;
	}

	if (swsCtx)
	{
		sws_freeContext(swsCtx);
		swsCtx = nullptr;
	}

	if (packet)
	{
		av_packet_free(&packet);
		packet = nullptr;
	}

	if (frame)
	{
		av_frame_free(&frame);
		frame = nullptr;
	}

	if (codecCtx)
	{
		avcodec_free_context(&codecCtx);
		codecCtx = nullptr;
	}

	if (fmtCtx)
	{
		avformat_close_input(&fmtCtx);
		fmtCtx = nullptr;
	}

	if (videoTexture != 0)
	{
		glDeleteTextures(1, &videoTexture);
		videoTexture = 0;
	}

	videoStream = nullptr;
	videoStreamIndex = -1;
	opened = false;

}

bool VideoPlayer::convertFrameToRGBA()
{
	if (!swsCtx || !frame || !rgbaPlanes[0])
		return false;

	sws_scale(
		swsCtx,
		frame->data,
		frame->linesize,
		0,
		videoHeight,
		rgbaPlanes,
		rgbaLinesize
	);

	return true;
}

bool VideoPlayer::decodeOneFrame()
{

	if (!opened)
	{
		std::cerr << "decodeOneFrame called but video not opened.\n";
		return false;
	}

	while (true)
	{
		int ret = av_read_frame(fmtCtx, packet);

		if (ret < 0)
		{
			std::cerr << "End of file or error reading frame.\n";
			return false;
		}

		if (packet->stream_index != videoStreamIndex)
		{
			av_packet_unref(packet);
			continue;
		}

		ret = avcodec_send_packet(codecCtx, packet);
		av_packet_unref(packet);

		if (ret < 0)
		{
			std::cerr << "Error sending packet to decoder.\n";
			return false;
		}

		while (true)
		{
			ret = avcodec_receive_frame(codecCtx, frame);
			if (ret == AVERROR(EAGAIN))
			{


				break;
			}
			else if (ret == AVERROR_EOF)
			{
				std::cerr << "Decoder signaled EOF.\n";
				return false;
			}
			else if (ret < 0)
			{
				std::cerr << "Error decoding frame.\n";
				return false;
			}

			int64_t pts = frame->best_effort_timestamp;
			if (pts == AV_NOPTS_VALUE)
				pts = frame->pts;

			if (pts != AV_NOPTS_VALUE)
			{
				double seconds = pts * av_q2d(videoStream->time_base);
				currentTime = seconds;  // <--- add this line

				std::cout << " pts: " << pts << " ("
					<< seconds << " sec)\n";
			}
			else
			{
				std::cout << " pts: N/A\n";
			}

			if (!convertFrameToRGBA())
			{
				std::cerr << "Failed to convert frame to RGBA.\n";
				return false;
			}

			glBindTexture(GL_TEXTURE_2D, videoTexture);

			glTexSubImage2D(
				GL_TEXTURE_2D,
				0,
				0, 0,
				videoWidth,
				videoHeight,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				rgbaPlanes[0]
			);

			glBindTexture(GL_TEXTURE_2D, 0);

			return true;
		}

	}

}

bool VideoPlayer::seekSeconds(double t)
{
	if (!opened || !videoStream)
		return false;

	// Clamp to [0, duration]
	if (videoDuration > 0.0)
	{
		if (t < 0.0) t = 0.0;
		if (t > videoDuration) t = videoDuration;
	}

	double tb = av_q2d(videoStream->time_base);
	int64_t targetPts = static_cast<int64_t>(t / tb);

	std::cout << "SeekSeconds requested: t=" << t
		<< " sec, targetPts=" << targetPts << "\n";

	// 1) Seek the demuxer
	int ret = av_seek_frame(fmtCtx, videoStreamIndex, targetPts,
		AVSEEK_FLAG_BACKWARD);
	if (ret < 0)
	{
		std::cerr << "Failed to seek to " << t << " sec\n";
		return false;
	}

	// 2) Flush decoder state so it's ready for new data
	avcodec_flush_buffers(codecCtx);

	// 3) Decode frames until we catch up to target time
	const double epsilon = 0.5 * (1.0 / videoFPS); // half-frame tolerance

	while (true)
	{
		if (!decodeOneFrame())
		{
			std::cerr << "Reached EOF while seeking.\n";
			return false;
		}

		double ct = currentTimeSeconds();
		if (ct + epsilon >= t)
		{
			std::cout << "Seeked, now at ~" << ct << " sec\n";
			break;
		}
	}

	return true;
}