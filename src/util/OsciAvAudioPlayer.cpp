//
//  OsciAvAudioPlayer.cpp
//  emptyExample
//
//  Created by Hansi on 13.07.15.
//
// a huge chunk of this file is based on the
// blog entry https://blinkingblip.wordpress.com/2011/10/08/decoding-and-playing-an-audio-stream-using-libavcodec-libavformat-and-libao/
// (had to make some adjustments for changes in ffmpeg
// and libavcodecs examples
// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decoding_encoding.c

#include "OsciAvAudioPlayer.h"
#include "Audio.h"
extern "C"{
	#include <libavutil/opt.h>
}
using namespace std;

#define die(msg) { thread->unlock(); unloadSound(); cerr << msg << endl; onEnd(); return false; }

OsciAvAudioPlayer::OsciAvAudioPlayer(){
	// default audio settings
	output_expected_buffer_size = 256;
	output_channel_layout = av_get_default_channel_layout(2);
	output_sample_rate = 44100;
	visual_sample_rate = 192000;
	visual_config_changed = false;
	output_num_channels = 2;
	output_config_changed = false; 
	volume = 1;
	wantsAsync = true;
	
	forceNativeFormat = false;
	
	isLooping = false;
	container = NULL; 
	decoded_frame = NULL;
	codec_context = NULL;
	buffer_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
	swr_context = NULL;
	swr_context192 = NULL;
	av_register_all();
	av_init_packet(&packet);
	unloadSound();
	
	thread = NULL;
}

OsciAvAudioPlayer::~OsciAvAudioPlayer(){
	unloadSound();
	if( thread != NULL ){
		thread->stopThread();
		thread = NULL;
	}
}

bool OsciAvAudioPlayer::loadSound(string fileName, bool stream){
	if( thread == NULL ){
		thread = new OsciAvAudioPlayerThread(*this);
		thread->startThread(); 
	}
	
	unloadSound();
	thread->lock();
	
	string fileNameAbs = ofToDataPath(fileName,true);
	const char * input_filename = fileNameAbs.c_str();
	// the first finds the right codec, following  https://blinkingblip.wordpress.com/2011/10/08/decoding-and-playing-an-audio-stream-using-libavcodec-libavformat-and-libao/
	container = 0;
	if (avformat_open_input(&container, input_filename, NULL, NULL) < 0) {
		die("Could not open file");
	}
 
	if (avformat_find_stream_info(container,NULL) < 0) {
		die("Could not find file info");
	}
 
	audio_stream_id = -1;
 
	// To find the first audio stream. This process may not be necessary
	// if you can gurarantee that the container contains only the desired
	// audio stream
	int i;
	for (i = 0; i < container->nb_streams; i++) {
		if (container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_id = i;
			break;
		}
	}
 
	if (audio_stream_id == -1) {
		die("Could not find an audio stream");
	}
 
	// Find the apropriate codec and open it
	codec_context = container->streams[audio_stream_id]->codec;
	

	if( forceNativeFormat ){
		output_sample_rate = codec_context->sample_rate;
		output_channel_layout = codec_context->channel_layout;
		if( output_channel_layout == 0 ){
			output_num_channels = codec_context->channels;
			output_channel_layout = av_get_default_channel_layout(output_num_channels);
		}
		else{
			output_num_channels = av_get_channel_layout_nb_channels( output_channel_layout );
		}
		cout << "native audio thing: " << output_sample_rate << "Hz / " << output_num_channels << " channels" << endl;
	}
	
	AVCodec* codec = avcodec_find_decoder(codec_context->codec_id);
	if (avcodec_open2(codec_context, codec,NULL)) {
		die("Could not find open the needed codec");
	}
	
	// from here on it's mostly following
	// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decoding_encoding.c
	//packet.data = inbuf;
	//packet.size = fread(inbuf, 1, AVCODEC_AUDIO_INBUF_SIZE, f);
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;

	swr_context = NULL;
	swr_context192 = NULL;
	isLoaded = true;
	isPlaying = true;
	fileType = STEREO; 

	// we continue here:
	decode_next_frame();
	duration = av_time_to_millis(container->streams[audio_stream_id]->duration);

	thread->unlock();
	
	return true;
}

bool OsciAvAudioPlayer::setupAudioOut( int numChannels, int sampleRate, bool inter ){
	if( numChannels != output_num_channels || sampleRate != output_sample_rate || inter != interpolate ){
		output_channel_layout = av_get_default_channel_layout(numChannels);
		output_sample_rate = sampleRate;
		output_num_channels = numChannels;
		interpolate = inter; 
		
		if( swr_context != NULL ){
			output_config_changed = true;
		}
	}
	
	return true;
}

bool OsciAvAudioPlayer::setupVisualSampleRate( int visualSampleRate ){
	if( visualSampleRate != visual_sample_rate ){
		visual_sample_rate = visualSampleRate;
		visual_config_changed = true;
	}
	return true; 
}



void OsciAvAudioPlayer::unloadSound(){
	if( !isLoaded ) return;
	thread->lock();
	
	len = 0;
	isLoaded = false;
	isPlaying = false;
	packet_data_size = 0;
	decoded_buffer_pos = 0;
	decoded_buffer_len = 0;
	next_seekTarget = -1;

	av_free_packet(&packet);
	
	if( decoded_frame ){
		av_frame_unref(decoded_frame);
		av_frame_free(&decoded_frame);
		decoded_frame = NULL;
	}
	
	if( container ){
		avcodec_close(codec_context);
		avformat_close_input(&container);
		avformat_free_context(container);
		av_free(container); 
		container = NULL;
		codec_context = NULL;
	}
	
	if( swr_context ){
		swr_close(swr_context);
		swr_free(&swr_context);
		swr_context = NULL;
	}

	if( swr_context192 ){
		swr_close(swr_context192);
		swr_free(&swr_context192);
		swr_context192 = NULL;
	}
	
	left192.clear();
	right192.clear();
	zMod192.clear();
	
	thread->unlock();
}

OsciAvAudioPlayerThread::OsciAvAudioPlayerThread( OsciAvAudioPlayer & player ) : player(player), isAsync(true){
}

void OsciAvAudioPlayerThread::threadedFunction(){
	// make sure we always have a bit of buffer ready
	while( isThreadRunning() ){
		lock();
		if( player.wantsAsync ){
			isAsync = true;
			if( player.next_seekTarget >= 0 ){
				player.mainOut.clear();
				player.left192.clear();
				player.right192.clear();
				player.zMod192.clear();
			}
			if( player.mainOut.totalLength < player.output_expected_buffer_size*4 && player.isLoaded ){
				float * buffer = new float[player.output_expected_buffer_size*2];
				int numSamples = player.internalAudioOut(buffer, player.output_expected_buffer_size, 2);
				if( numSamples > 0 ){
					player.mainOut.append( buffer, 2*numSamples );
				}
				delete buffer;
			}
		}
		else{
			isAsync = false;
		}
		unlock();
		sleep(1);
		yield();
	}
}

int OsciAvAudioPlayer::audioOut(float *output, int bufferSize, int nChannels){
	if( wantsAsync ){
		output_expected_buffer_size = bufferSize;
		if( nChannels != 2 ){
			// this is fucked up!
			return 0;
		}
		else if( mainOut.totalLength > 0 ){
			// i'm an idiot, and that's why we have to do this!
			mainOut.playbackIndex = 0;
			mainOut.play();
			int res = mainOut.addTo(output, 1, 2*bufferSize);
			mainOut.peel(res);
			return res;
		}
		return 0;
	}
	else{
		return 0;
	}
}

int OsciAvAudioPlayer::audioOutSync(float *output, int bufferSize, int nChannels){
	return internalAudioOut(output, bufferSize, nChannels);
}

int OsciAvAudioPlayer::internalAudioOut(float *output, int bufferSize, int nChannels){
	if( !isLoaded ){ return 0; }
	
	if( next_seekTarget >= 0 ){
		mainOut.clear();
		left192.clear();
		right192.clear();
		zMod192.clear();
		//av_seek_frame(container,-1,next_seekTarget,AVSEEK_FLAG_ANY);
		avformat_seek_file(container,audio_stream_id,0,next_seekTarget,next_seekTarget,AVSEEK_FLAG_ANY);
		next_seekTarget = -1;
		avcodec_flush_buffers(codec_context);
		decode_next_frame();
	}
	
	if( !isPlaying ){ return 0; }
	
	
	int max_read_packets = 4;
	// number of samples read per channel (up to bufferSize)
	int num_samples_read = 0;
	
	if( decoded_frame == NULL ){
		decode_next_frame();
	}
	
	while( decoded_frame != NULL && max_read_packets > 0 ){
		max_read_packets --;
		
		int missing_samples = bufferSize*nChannels - num_samples_read;
		int available_samples = decoded_buffer_len - decoded_buffer_pos;
		if( missing_samples > 0 && available_samples > 0 ){
			int samples = min( missing_samples, available_samples );
			// uh...
			//samples -= samples % 4; 

			if( volume != 0 ){
				memcpy(output+num_samples_read, decoded_buffer+decoded_buffer_pos, samples*sizeof(float) );
			}
			
			if( volume != 1 && volume != 0 ){
				for( int i = 0; i < samples; i++ ){
					output[i+num_samples_read] *= volume;
				}
			}
			
			int samples192 = samples - (samples % numChannels192);
			decoded_buffer_pos += samples;
			num_samples_read += samples;

			// find copy points in 192k buffer
			int a = (decoded_buffer_pos-samples)*(int64_t)visual_sample_rate/output_sample_rate;
			int b = (decoded_buffer_pos)*(int64_t)visual_sample_rate/output_sample_rate;
			a *= numChannels192 / 2; 
			b *= numChannels192 / 2; 
			a = a - (a%numChannels192);
			b = MIN(b - (b%numChannels192), decoded_buffer_len192);
			//cout << a << "\t" << b << "\t\t" << decoded_buffer_len192 << "\t\t\t" << samples << "\t" << decoded_buffer_len << endl; 
			if( b-a > 0 ){
				switch(fileType){
					case STEREO_ZMODULATED:
						zMod192.append( decoded_buffer192+2+a, (b-a)/3, 3);
						left192.append( decoded_buffer192+a, (b-a)/3, 3);
						right192.append( decoded_buffer192+1+a, (b-a)/3, 3);
						break;
					default:
						left192.append( decoded_buffer192+a, (b-a)/2, 2);
						right192.append( decoded_buffer192+1+a, (b-a)/2, 2);
						break;
				}
			}
		}
		
		if( num_samples_read >= bufferSize*nChannels ){
			return bufferSize;
		}
		else{
			decode_next_frame();
		}
	}
	
	return num_samples_read/nChannels;
}

bool OsciAvAudioPlayer::decode_next_frame(){
	av_free_packet(&packet);
	int res = av_read_frame(container, &packet);
	bool didRead = res >= 0;
	
	if( didRead ){
		int got_frame = 0;
		if (!decoded_frame) {
			if (!(decoded_frame = av_frame_alloc())) {
				fprintf(stderr, "Could not allocate audio frame\n");
				return false;
			}
		}
		else{
			av_frame_unref(decoded_frame);
		}
		
		len = avcodec_decode_audio4(codec_context, decoded_frame, &got_frame, &packet);
		
		if (len < 0) {
			// no data
			return false;
		}
		
		if( packet.stream_index != audio_stream_id ){
			return false;
		}

		if (got_frame) {
			lastPts = av_frame_get_best_effort_timestamp(decoded_frame);
//			lastPts = decoded_frame->pkt_pts;
			
			int frame_channels = av_frame_get_channels(decoded_frame);
			if(frame_channels != swr_context_channels ){
				output_config_changed = true;
			}
			if( swr_context != NULL && output_config_changed ){
				output_config_changed = false;
				if( swr_context ){
					swr_close(swr_context);
					swr_free(&swr_context);
					swr_context = NULL;
				}
			}
			
			
			if( swr_context == NULL ){
				int input_channel_layout = av_frame_get_channel_layout(decoded_frame);
				if( input_channel_layout == 0 ){
					input_channel_layout = av_get_default_channel_layout( av_frame_get_channels(decoded_frame) );
				}
				swr_context_channels = av_frame_get_channels(decoded_frame);
				swr_context = swr_alloc_set_opts(NULL,
												 output_channel_layout, AV_SAMPLE_FMT_FLT, output_sample_rate,
												 input_channel_layout, (AVSampleFormat)decoded_frame->format, av_frame_get_sample_rate(decoded_frame),
												 0, NULL);
				swr_init(swr_context);
				
				if (!swr_context){
					fprintf(stderr, "Could not allocate resampler context\n");
					return false;
				}
				
				int next_v_rate = output_sample_rate;
				if( next_v_rate != visual_sample_rate ){
					visual_sample_rate = max(192000,next_v_rate);
					visual_config_changed = true;
				}
			}
			
			if( swr_context192 != NULL && visual_config_changed ){
				swr_close(swr_context192);
				swr_free(&swr_context192);
				swr_context192 = NULL;
			}
			
			switch(av_frame_get_channels(decoded_frame)){
				case 1:
					// mono file, make it stereo!
					numChannels192 = 2;
					fileType = MONO;
					break;
				case 2:
					// stereo file, all is well
					numChannels192 = 2;
					fileType = STEREO;
					break;
				case 3:
					// stereo+zmod
					numChannels192 = 3;
					fileType = STEREO_ZMODULATED;
					break;
				case 4:
					// quad (2x stereo)
					numChannels192 = 4;
					fileType = QUAD;
					break;
				default:
					// no clue, fuck it... stereo it is!
					numChannels192 = 2;
					fileType = STEREO;
					break;
			}
			
			if( swr_context192 == NULL ){
				visual_config_changed = false;
				int input_channel_layout = av_frame_get_channel_layout(decoded_frame);
				if( input_channel_layout == 0 ){
					input_channel_layout = av_get_default_channel_layout( av_frame_get_channels(decoded_frame) );
				}
				swr_context192 = swr_alloc_set_opts(NULL,
												 av_get_default_channel_layout(numChannels192), AV_SAMPLE_FMT_FLT, visual_sample_rate,
												 input_channel_layout, (AVSampleFormat)decoded_frame->format, av_frame_get_sample_rate(decoded_frame),
												 0, NULL);

				//enable these two to disable interpolation
				//if(!interpolate){
				//	av_opt_set_int(swr_context192, "filter_size", 4, 0);
				//	av_opt_set_int(swr_context192, "linear_interp", 1, 0);
				//}

				
				//				av_opt_set_int(swr_context192, "dither_scale", 0, 0);
				swr_init(swr_context192);
				if (!swr_context192){
					fprintf(stderr, "Could not allocate resampler-192k context\n");
					return false;
				}
			}
			
			
			/* if a frame has been decoded, resample to desired rate */
			uint8_t * out192 = (uint8_t*)decoded_buffer192;
			int samples_converted192 = swr_convert(swr_context192,
												   (uint8_t**)&out192, AVCODEC_MAX_AUDIO_FRAME_SIZE/4,
												   (const uint8_t**)decoded_frame->extended_data, decoded_frame->nb_samples);
			
			decoded_buffer_len192 = samples_converted192*numChannels192;
			decoded_buffer_pos192 = 0;


			int samples_per_channel = AVCODEC_MAX_AUDIO_FRAME_SIZE/output_num_channels;
			//samples_per_channel = 512;
			uint8_t * out = (uint8_t*)decoded_buffer;
			int samples_converted = swr_convert(swr_context,
												(uint8_t**)&out, samples_per_channel,
												(const uint8_t**)decoded_frame->extended_data, decoded_frame->nb_samples);
			decoded_buffer_len = samples_converted*output_num_channels;
			decoded_buffer_pos = 0;
			
		}

		packet.size -= len;
		packet.data += len;
//		packet->dts =
//		packet->pts = AV_NOPTS_VALUE;
		
		return true;
	}
	else{
		// no data read...
		packet_data_size = 0;
		decoded_buffer_len = 0;
		decoded_buffer_pos = 0;
		if( isLooping ){
			avformat_seek_file(container,audio_stream_id,0,0,0,AVSEEK_FLAG_ANY);
			avcodec_flush_buffers(codec_context);
			decode_next_frame();
		}
		else if(isPlaying){
			isPlaying = false;
			onEnd();
		}
		
		return false;
	}
}

unsigned long long OsciAvAudioPlayer::av_time_to_millis( int64_t av_time ){
	return av_rescale(1000*av_time,(uint64_t)container->streams[audio_stream_id]->time_base.num,container->streams[audio_stream_id]->time_base.den);
	//alternative:
	//return av_time*1000*av_q2d(container->streams[audio_stream_id]->time_base);
}

int64_t OsciAvAudioPlayer::millis_to_av_time( unsigned long long ms ){
	//TODO: fix conversion
/*	int64_t timeBase = (int64_t(codec_context->time_base.num) * AV_TIME_BASE) / int64_t(codec_context->time_base.den);
	int64_t seekTarget = int64_t(ms) / timeBase;*/
	return av_rescale(ms,container->streams[audio_stream_id]->time_base.den,(uint64_t)container->streams[audio_stream_id]->time_base.num)/1000;
}



void OsciAvAudioPlayer::setPositionMS(int ms){
	next_seekTarget = millis_to_av_time(ms);
}

int OsciAvAudioPlayer::getPositionMS(){
	if( !isLoaded ) return 0;
	return av_time_to_millis( lastPts );
}

float OsciAvAudioPlayer::getPosition(){
	return duration == 0? 0 : (getPositionMS()/(float)duration);
}

void OsciAvAudioPlayer::setPosition(float percent){
	if(duration>0&&isLoaded) setPositionMS(percent*duration);
}

void OsciAvAudioPlayer::stop(){
	isPlaying =false;
}

void OsciAvAudioPlayer::play(){
	if( isLoaded ){
		isPlaying = true;
	}
}

void OsciAvAudioPlayer::setLoop(bool loop){
	isLooping = loop;
}

void OsciAvAudioPlayer::setVolume(float vol){
	this->volume = vol;
}

string OsciAvAudioPlayer::getMetadata( string key ){
	if( container != NULL ){
		AVDictionaryEntry * entry = av_dict_get(container->metadata, key.c_str(), NULL, 0);
		if( entry == NULL ) return "";
		else return string(entry->value);
	}
	else{
		return "";
	}
}

map<string,string> OsciAvAudioPlayer::getMetadata(){
	map<string,string> meta;
	AVDictionary * d = container->metadata;
	AVDictionaryEntry *t = NULL;
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))!=0){
		meta[string(t->key)] = string(t->value);
	}
	
	return meta; 
}

void OsciAvAudioPlayer::beginSync( int bufferSize ){
	if( !isLoaded ) return;
	
	wantsAsync = false;
	while( thread->isAsync ){
		ofSleepMillis(10);
	}
	
	left192.clear();
	right192.clear();
	zMod192.clear();
	mainOut.clear();
	
	output_expected_buffer_size = bufferSize;
}

void OsciAvAudioPlayer::endSync(){
	if( !isLoaded ) return;
	wantsAsync = true;
	
	while( !thread->isAsync ){
		ofSleepMillis(10);
	}
}