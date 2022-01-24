//
//  OsciVideoWriter.hpp
//  Oscilloscope
//
//  Created by Hansi on 06.06.18.
//
//
//

/// \brief Class to write video (in sync, so it's very slow!)

#ifndef OsciVideoWriter_hpp
#define OsciVideoWriter_hpp

#include <string>
#include "ofPixels.h"

struct AVFormatContext;
struct AVOutputFormat;
struct AVStream;
struct AVFrame;
struct AVPacket;
struct SwsContext;

class OsciVideoWriter{
public:
	
	struct WorkData{
		
		AVFormatContext* out_ctx = nullptr;
		AVOutputFormat* out_fmt = nullptr;
		AVStream* out_video_st = nullptr;
		AVStream* out_audio_st = nullptr;

		// temp audio buffers
		uint8_t out_audio_buffer[32 * 2048 * 8] = { 0 };
		size_t out_audio_buffer_offset = 0;
		
		// number of written audio frames
		int64_t audio_pts = 0;
		int64_t audio_pts_pkt = 0;

		// temp video buffers
		SwsContext* sws_ctx;
		AVFrame* in_frame = nullptr;
		AVFrame* out_frame = nullptr;

		// number of written video frames
		int64_t video_frame_num = 0;
	};
	
	struct OutputConfig{
		int video_framerate;
		int video_width;
		int video_height;
		
		int audio_sample_rate;
		int audio_n_channels;
	};
	
	OsciVideoWriter();
	~OsciVideoWriter();
	
	/// \brief Adds the video file. Returns true on success
	bool open(std::string outputFile, const OutputConfig & config);
	
	/// \brief Adds a video frame from pixels. If the pixel size doesn't match, it fails.
	void addVideoFrame(const ofPixels & pixels);
	
	/// \brief Adds interleaved samples
	void addAudioFrame(float * samples, int numSamples);
	
	/// \brief Closes the file. Always call this, lots of file info must be written at the end. 
	void close();
	
	bool isOpen();
	
	
private:
	OutputConfig config;
	bool is_open = false;
	
	WorkData wd;
};

#endif /* OsciVideoWriter_hpp */
