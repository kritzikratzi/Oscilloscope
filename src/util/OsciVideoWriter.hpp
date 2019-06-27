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

class OsciVideoWriter{
public:
	
	
	struct OutputConfig{
		int video_framerate;
		int video_width;
		int video_height;
		
		int audio_sample_rate;
		int audio_n_channels;
	};
	
	
	/// \brief Adds the video file. Returns true on success
	bool open(string outputFile, const OutputConfig & config);
	
	/// \brief Adds a video frame from pixels. If the pixel size doesn't match, it fails.
	bool addVideoFrame(const ofPixels & pixels);
	
	/// \brief Adds interleaved samples
	bool addAudioFrame(const float * sample, int numSamples);
	
	/// \brief Closes the file. Always call this, lots of file info must be written at the end. 
	void close();
	
	
private:
	OutputConfig config;
	
	AVFormatContext * out_ctx;
	AVOutputFormat* out_fmt;
	AVStream * out_video_st;
	AVStream * out_audio_st;
	AVFrame * out_frame;
	uint8_t * out_pixels;
};

#endif /* OsciVideoWriter_hpp */
