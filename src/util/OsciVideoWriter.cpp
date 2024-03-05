//
//  OsciVideoWriter.cpp
//  Oscilloscope
//
//  Created by Hansi on 06.06.18.
//
//

#include "OsciVideoWriter.hpp"
#include "ofMain.h"

#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER
#define AVFMT_RAWPICTURE 0x0020

extern "C"{
	#include <stdlib.h>
	#include <stdio.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/samplefmt.h>
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
}
#include "ofxAvUtils.h"

#pragma mark general purpose stuff (implementation at the end of the file)

AVStream * add_audio_stream(AVFormatContext *oc, AVCodecID codec_id, int sample_rate, int num_channels);
AVStream * add_video_stream(AVFormatContext* oc, AVCodecID codec_id, int width, int height, int fps);
bool init_sws(SwsContext** sws_ctx_ptr, AVFrame** in_frame_ptr, AVPixelFormat in_format, int in_width, int in_height, AVFrame** out_frame_ptr, AVPixelFormat out_format, int out_width, int out_height);

void open_video(AVFormatContext *oc, AVStream *st);
void prepare_video_frame(SwsContext* sws_ctx, uint8_t* pixels, AVFrame* in_frame, AVFrame* out_frame);
void write_video_frame(AVFormatContext * out_ctx, AVStream* st, AVFrame* out_frame, int64_t * video_frame_num);
void close_video(AVFormatContext *oc, AVStream *st);

void open_audio(AVFormatContext *oc, AVStream *st);
void write_audio_frames(AVFormatContext *oc, AVStream *st, OsciVideoWriter::WorkData & wd, float * samples, int num_samples, bool force_write);
void close_audio(AVFormatContext *oc, AVStream *st);

#pragma mark OsciVideoWriter implementation

OsciVideoWriter::OsciVideoWriter(){
}

OsciVideoWriter::~OsciVideoWriter(){

}

bool OsciVideoWriter::open(std::string outputFile, const OsciVideoWriter::OutputConfig & config){
	close();
	
	outputFile = ofToDataPath(outputFile,true);
	

	// use m4a instead mp4 to support alac
	// https://github.com/nschlia/ffmpegfs/issues/37
	wd.out_fmt = av_guess_format( "mov", nullptr, nullptr );
	if( !wd.out_fmt ){
		close();
		return false;
	}
	
	wd.out_ctx = avformat_alloc_context();
	if( !wd.out_ctx ){
		fprintf(stderr, "Memory error\n");
		close();
		return false;
	}
	
	wd.out_ctx->oformat = wd.out_fmt;
	wd.out_video_st = nullptr;
	wd.out_audio_st = nullptr;

	//out_pix_fmt = AV_PIX_FMT_YUV420P;
	wd.out_video_st = add_video_stream(wd.out_ctx, AV_CODEC_ID_H264, config.video_width, config.video_height, config.video_framerate );
	wd.out_audio_st = add_audio_stream(wd.out_ctx, AV_CODEC_ID_ALAC, config.audio_sample_rate, config.audio_n_channels);
	
	open_video(wd.out_ctx, wd.out_video_st);
	open_audio(wd.out_ctx, wd.out_audio_st);

	// not sure why, but here it's
	// AV_PIX_FMT_BGR32 not AV_PIX_FMT_RGB32
	init_sws(&wd.sws_ctx, &wd.in_frame, AV_PIX_FMT_BGR32, config.video_width, config.video_height , &wd.out_frame, wd.out_video_st->codec->pix_fmt, wd.out_video_st->codec->width, wd.out_video_st->codec->height);

	if (!(wd.out_fmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&wd.out_ctx->pb, outputFile.c_str(), AVIO_FLAG_WRITE) < 0) {
			fprintf(stderr, "Could not open '%s'\n", outputFile.c_str());
			close();
			return false;
		}
	}

	// write stream header
	avformat_write_header(wd.out_ctx,NULL);
	is_open = true;
	return true;
}

void OsciVideoWriter::close(){
	is_open = false;
	
	if( wd.out_fmt != nullptr ){
		int i;
		
		// write any audio buffers still hanging around
		write_audio_frames(wd.out_ctx, wd.out_audio_st, wd, nullptr, 0, true);
		
		/* write the trailer, if any.  the trailer must be written
		 * before you close the CodecContexts open when you wrote the
		 * header; otherwise write_trailer may try to use memory that
		 * was freed on av_codec_close() */
		av_write_trailer(wd.out_ctx);
		
		/* close each codec */
		if (wd.out_video_st)
			close_video(wd.out_ctx, wd.out_video_st);
		if (wd.out_audio_st)
			close_audio(wd.out_ctx, wd.out_audio_st);
		
		if(wd.out_frame){
			av_free(wd.out_frame->data[0]);
			av_free(wd.out_frame);
		}
		if(wd.in_frame){
			av_free(wd.in_frame->data[0]);
			av_free(wd.in_frame);
		}
		sws_freeContext(wd.sws_ctx);

		
		/* free the streams */
		//GOOD GOD!
		for(i = 0; i < wd.out_ctx->nb_streams; i++) {
			av_freep(&wd.out_ctx->streams[i]->codec);
			av_freep(&wd.out_ctx->streams[i]);
		}
		
		if (!(wd.out_fmt->flags & AVFMT_NOFILE)) {
			/* close the output file */
			avio_closep(&wd.out_ctx->pb);
		}
		
		wd = WorkData();
	}
}

bool OsciVideoWriter::isOpen(){
	return is_open; 
}

void OsciVideoWriter::addAudioFrame(float *samples, int numSamples){
	write_audio_frames(wd.out_ctx, wd.out_audio_st, wd, samples, 2*numSamples, false);
}

void OsciVideoWriter::addVideoFrame(const ofPixels &pixels){
	prepare_video_frame(wd.sws_ctx, const_cast<unsigned char*>(pixels.getData()), wd.in_frame, wd.out_frame);
	write_video_frame(wd.out_ctx, wd.out_video_st, wd.out_frame, &wd.video_frame_num);
}





#pragma mark FFmpeg: Audio Snippets


AVStream * add_audio_stream(AVFormatContext *out_ctx, AVCodecID codec_id, int sample_rate, int num_channels)
{
	AVCodecContext *c;
	AVStream *st;
	
	//TODO: is 0 okay?
	AVCodec *codec = avcodec_find_encoder(codec_id);
	if (!(codec)) {
		fprintf(stderr, "Could not find encoder for '%s'\n",
				avcodec_get_name(codec_id));
		exit(1);
	}
	st = avformat_new_stream(out_ctx, codec); // put codec here?
	if (!st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}
	
	st->id = out_ctx->nb_streams-1;
	c = st->codec;
	c->codec_id = codec_id;
	c->codec_type = AVMEDIA_TYPE_AUDIO;
	
	c->sample_rate = sample_rate;
	c->channels = num_channels;
	c->channel_layout = av_get_default_channel_layout(num_channels);
	st->time_base = AVRational{1,sample_rate};
	
	if(c->codec_id == AV_CODEC_ID_AAC){
		// let's compress this ^^
		c->sample_fmt = AV_SAMPLE_FMT_FLTP;
		c->bit_rate = 320000; // 320kb
		c->rc_min_rate = 320000; // 320kb
		c->rc_max_rate = 320000; // 320kb
		c->qmin = 0;
		c->qmax = 1;
		c->global_quality = 2;
		c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		av_opt_set(c->priv_data, "aac_coder", "twoloop", 0);
		av_opt_set(c->priv_data, "aac_ms", "0", 0);
		av_opt_set(c->priv_data, "aac_is", "0", 0);
		av_opt_set(c->priv_data, "aac_pns", "0", 0);
		av_opt_set(c->priv_data, "aac_tns", "0", 0);
		av_opt_set(c->priv_data, "aac_ltp", "0", 0);
		av_opt_set(c->priv_data, "aac_pred", "0", 0);
		av_opt_set(c->priv_data, "aac_pce", "0", 0);
//		c->profile = FF_PROFILE_AAC_MAIN;
		c->compression_level = 0;
	}
	else if(c->codec_id == AV_CODEC_ID_ALAC){
		c->sample_fmt = AV_SAMPLE_FMT_S16P;
		c->bit_rate = 320000; // 320kb
		c->rc_min_rate = 320000; // 320kb
		c->rc_max_rate = 320000; // 320kb
		c->qmin = 0;
		c->qmax = 1;
		c->global_quality = 2;
//		c->profile = FF_PROFILE_AAC_MAIN;
		c->strict_std_compliance = FF_COMPLIANCE_STRICT;
		c->compression_level = 0;
	}
	else if(c->codec_id == AV_CODEC_ID_PCM_F32LE){
		c->sample_fmt = AV_SAMPLE_FMT_FLT;
	}
	else{
		cerr << "unsupported audio codec: " << c->codec_id << endl;
		exit(1);
	}
	
	// some formats want stream headers to be separate
	if(out_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	
	return st;
}

// avcodec_get_frame_defaults is gone :(
void VT_avcodec_get_frame_defaults(AVFrame *frame)
{
	if (frame->extended_data != frame->data && 0)
		av_freep(&frame->extended_data);
	
	memset(frame, 0, sizeof(AVFrame));
	av_frame_unref(frame);
}


void open_audio(AVFormatContext *oc, AVStream *st)
{
	AVCodecContext *c;
	AVCodec *codec;
	
	c = st->codec;
	/* find the audio encoder */
	codec = avcodec_find_encoder(c->codec_id);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}
	
	/* open it */
	if (avcodec_open2(c, codec,NULL) < 0) {
		fprintf(stderr, "could not open codec\n");
		return;
	}
}

void write_audio_frames( AVFormatContext *out_ctx, AVStream *st, OsciVideoWriter::WorkData & wd, float * buffer, int totalLen, bool forceWrite ){
	AVCodecContext *c = st->codec;
	uint8_t * out_audio_buffer = wd.out_audio_buffer;
	size_t & out_audio_buffer_offset = wd.out_audio_buffer_offset;
	int64_t & audio_pts = wd.audio_pts;
	int64_t & audio_pts_pkt = wd.audio_pts_pkt;

rinseAndRepeat:
	
	
	int requiredLength = c->frame_size>0?(c->frame_size*st->codec->channels):totalLen;
	int remainingBufferLength = requiredLength-out_audio_buffer_offset;
	int len = std::min(remainingBufferLength,totalLen);
	int out_num_channels = st->codecpar->channels;
	int samplerate = st->codecpar->sample_rate;
	
	// the input here is an interleaved float buffer.
	// let's fix this up ^^
	int lenPerChannel = len/st->codecpar->channels;
	switch((AVSampleFormat)st->codecpar->format){
		case AV_SAMPLE_FMT_FLTP:{
			float * dest = (float*)(out_audio_buffer);
			int idx_offset = out_audio_buffer_offset/out_num_channels;
			int buffer_len_per_channel = requiredLength/out_num_channels;
			for(int ch = 0; ch < out_num_channels; ch++){
				for(int idx = 0; idx < lenPerChannel; idx++){
					dest[idx + idx_offset + ch*buffer_len_per_channel] = buffer[out_num_channels*idx + ch];
				}
			}
			break;
		}
		case AV_SAMPLE_FMT_S32P:{
			int32_t * idest = (int32_t*)(out_audio_buffer);
			int idx_offset = out_audio_buffer_offset/out_num_channels;
			int buffer_len_per_channel = requiredLength/out_num_channels;
			for(int ch = 0; ch < out_num_channels; ch++){
				for(int idx = 0; idx < lenPerChannel; idx++){
					idest[idx + idx_offset + ch*buffer_len_per_channel] = ofMap(buffer[out_num_channels*idx + ch],-1,1,-2147483647,2147483647,true);
				}
			}
			break;
		}
		case AV_SAMPLE_FMT_S16P:{
			int16_t * sdest = (int16_t*)(out_audio_buffer);
			int idx_offset = out_audio_buffer_offset/out_num_channels;
			int buffer_len_per_channel = requiredLength/out_num_channels;
			for(int ch = 0; ch < out_num_channels; ch++){
				for(int idx = 0; idx < lenPerChannel; idx++){
					sdest[idx + idx_offset + ch*buffer_len_per_channel] = ofMap(buffer[out_num_channels*idx + ch],-1,1,-32767,32767,true);
				}
			}
			break;
		}
		case AV_SAMPLE_FMT_FLT:{
			memcpy(out_audio_buffer+out_audio_buffer_offset, buffer, len*sizeof(float));
			break;
		}
		default:
			cout << "sample format not supported: " << av_get_sample_fmt_name((AVSampleFormat)st->codecpar->format) << endl;
			return;
	}
	
	out_audio_buffer_offset += len;
	if((requiredLength > 0 && out_audio_buffer_offset>=requiredLength) || (forceWrite && out_audio_buffer_offset>0)){
		int write_len = out_audio_buffer_offset;
		out_audio_buffer_offset = 0;
		if(write_len>requiredLength){
			cerr << "too much data in the output buffer. " << endl;
		}

		AVFrame * frm = av_frame_alloc();
		int got_output;
		int ret;
		VT_avcodec_get_frame_defaults(frm);
		frm->nb_samples     = write_len/out_num_channels;
		frm->format         = c->sample_fmt;
		frm->channel_layout = c->channel_layout;

		int buffer_size = av_samples_get_buffer_size(NULL, c->channels,
												 frm->nb_samples, c->sample_fmt,
												 0);
		ret = avcodec_fill_audio_frame(frm, c->channels, c->sample_fmt,
									   (const uint8_t*)out_audio_buffer, buffer_size, 0);

		AVPacket pkt;
		av_init_packet(&pkt);
		if(c->codec_id == AV_CODEC_ID_PCM_F32LE){
			pkt.data = out_audio_buffer;
			pkt.size = buffer_size;
			got_output = true;
		}
		else{
			pkt.data = NULL; // packet data will be allocated by the encoder
			pkt.size = 0;
			ret = avcodec_encode_audio2(c, &pkt, frm, &got_output);
		}
		
		audio_pts += requiredLength/out_num_channels; // keep incrementing the encoding pts
		
		if(got_output){
			pkt.pts = pkt.dts = av_rescale_q(audio_pts_pkt, AVRational{1,samplerate}, c->time_base);
			audio_pts_pkt = audio_pts; // timestamp for next packet
			// cout << "pts = dts = " << pkt.dts << endl;
			pkt.flags |= AV_PKT_FLAG_KEY;
			pkt.stream_index= st->index;
	//		pkt.data= audio_outbuf;
			
			if (av_interleaved_write_frame(out_ctx, &pkt) != 0) {
				fprintf(stderr, "Error while writing audio frame\n");
				exit(1);
			}
			av_packet_unref(&pkt);
		}
	}
	
	buffer += len;
	totalLen -= len;
	if(totalLen>0){
		goto rinseAndRepeat;
	}
}

void close_audio(AVFormatContext *oc, AVStream *st)
{
	avcodec_close(st->codec);
}



#pragma mark FFmpeg: Video Snippets

AVStream * add_video_stream( AVFormatContext *oc, AVCodecID codec_id, int width, int height, int fps ){
	AVCodecContext *c;
	AVStream *st;
	AVCodec* out_codec = avcodec_find_encoder(codec_id);

	if (!(out_codec)) {
		fprintf(stderr, "Could not find encoder for '%s'\n",
				avcodec_get_name(codec_id));
		exit(1);
	}
	st = avformat_new_stream(oc, out_codec);
	if (!st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}
	
	c = st->codec;
	st->id = oc->nb_streams-1;
	c->codec_id = codec_id;
	c->codec_type = AVMEDIA_TYPE_VIDEO;
	c->colorspace = AVCOL_SPC_BT709;
	c->color_range = AVCOL_RANGE_MPEG;
	c->gop_size = 0; /* emit one intra frame every frame */
	c->qmin=0;
	c->qmax=0;
	c->qcompress=0;
	c->qblur=0;
	/* put sample parameters */
	int64_t mbits = 20; 
	c->bit_rate = 1000000l * mbits; // 20 megabit
	
	if(codec_id == AV_CODEC_ID_H264){

		int64_t rate = mbits *1000.0 * 9.5;
		c->bit_rate = rate;
		c->qmin=31;
		c->qmax=31;
		c->gop_size = 1;

		int res;
		res = av_opt_set_int(c->priv_data, "max_nal_size", 32*4096, 0);
		av_opt_set(c->priv_data, "profile", "baseline", AV_OPT_SEARCH_CHILDREN);
		av_opt_set(c->priv_data, "level", "5.1", AV_OPT_SEARCH_CHILDREN);
		av_opt_set(c->priv_data, "preset", "slow", AV_OPT_SEARCH_CHILDREN);
		av_opt_set(c->priv_data, "crf",  "10", AV_OPT_SEARCH_CHILDREN);
	}
	
	/* resolution must be a multiple of two */
	c->width = width;
	c->height = height;

	AVRational timebase{ 1,1000*fps };
	c->time_base = timebase;
	st->time_base = timebase;
	st->r_frame_rate = { fps,1 };
	
	
	//TODO: what format do we want?
	c->pix_fmt = AV_PIX_FMT_YUV420P;
	
	// some formats want stream headers to be separate
	if(oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	
	return st;
}



void open_video(AVFormatContext *oc, AVStream *st)
{
	AVCodecContext *c = st->codec;
	/* find the video encoder */
	AVCodec * codec = avcodec_find_encoder(st->codec->codec_id);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}
	
	/* open the codec */
	if (avcodec_open2(c, codec,NULL) < 0) {
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}
}

void prepare_video_frame(SwsContext* sws_ctx, uint8_t* pixels, AVFrame* in_frame, AVFrame* out_frame) {
	//picture_rgb->data[0] = (uint8_t*) pixels;
	//avpicture_fill((AVPicture*)picture_rgb, pixels, PIX_FMT_RGB24, width, height);
	//TODO: why does this not crash?
	const size_t N = in_frame->width * in_frame->height * (size_t)4; // num color channels
	for (size_t i = 0; i < N; i++) {
		in_frame->data[0][i] = pixels[i];
	}

	sws_scale(sws_ctx,
		// src slice / src stride
		in_frame->data, in_frame->linesize,
		// src slice y / src slice h
		0, in_frame->height,
		// destinatin / destination stride
		out_frame->data, out_frame->linesize);
}


void write_video_frame(AVFormatContext * out_ctx, AVStream* st, AVFrame* out_frame, int64_t * video_frame_num)
{
	int out_size, ret;
	AVCodecContext *c;
	long a = ofGetSystemTime();
	c = st->codec;
	

	out_frame->pts = (*video_frame_num) / av_q2d(st->time_base) / av_q2d(st->r_frame_rate);
	int64_t pkt_duration = (1.0) / av_q2d(st->time_base) / av_q2d(st->r_frame_rate);
	(*video_frame_num)++;

	//av_packet_rescale_ts(picture, video_stream->time_base, out_video_st->time_base);
	if (out_ctx->oformat->flags & AVFMT_RAWPICTURE) {
		/* raw video case. The API will change slightly in the near
		 futur for that */
		AVPacket pkt;
		av_init_packet(&pkt);
		
		pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index= st->index;
		pkt.data = (uint8_t*)out_frame;
		pkt.size= sizeof(AVPicture);
		
		ret = av_interleaved_write_frame(out_ctx, &pkt);
	} else {
		/* encode the image */
		a = ofGetSystemTime();
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);
		/* encode the image */
		int got_packet;
		ret = avcodec_encode_video2(c, &pkt, out_frame, &got_packet);
		if (ret < 0) {
			fprintf(stderr, "Error encoding video frame: %s\n", ofxAvUtils::errorString(ret).c_str());
			exit(1);
		}
		
		if( got_packet ){
			pkt.pts = out_frame->pts;
			pkt.stream_index = st->index;
			pkt.duration = pkt_duration;
			av_interleaved_write_frame(out_ctx, &pkt);
			return;
		}

		a = ofGetSystemTime();
		
		
	}
	if (ret != 0) {
		fprintf(stderr, "Error while writing video frame\n");
		exit(1);
	}
}

void close_video(AVFormatContext *oc, AVStream *st)
{
	avcodec_close(st->codec);
}



bool init_sws(SwsContext** sws_ctx_ptr, AVFrame** in_frame_ptr, AVPixelFormat in_format, int in_width, int in_height, AVFrame** out_frame_ptr, AVPixelFormat out_format, int out_width, int out_height) {

	AVFrame* out_frame = av_frame_alloc();
	*out_frame_ptr = out_frame;
	if (!out_frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		return false;
	}
	out_frame->format = out_format;
	out_frame->width = out_width;
	out_frame->height = out_height;

	int ret = av_image_alloc(out_frame->data, out_frame->linesize, out_width, out_height,
		out_format, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		return false;
	}

	AVFrame* in_frame = av_frame_alloc();
	*in_frame_ptr = in_frame;
	if (!in_frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		return false;
	}
	in_frame->format = in_format;
	in_frame->width = in_width;
	in_frame->height = in_height;

	
	
	ret = av_image_alloc(in_frame->data, in_frame->linesize, in_width, in_height,
		in_format, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		return false;
	}


	SwsContext* convert_ctx = sws_getContext(
		// source
		in_width, in_height, in_format,
		// dest
		out_width, out_height, out_format,
		// flags / src filter / dest filter / param
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	*sws_ctx_ptr = convert_ctx;

	return true;
}

