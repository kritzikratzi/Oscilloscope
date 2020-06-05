//
// Audio.h
//
// Initially created by Hansi on 14.06.14.
// V1.8, 17.8.2019. got rid of poco dependency
// V1.7, 10.11.2017: added playFrom, fixed a copy bug in the zero-padding in copyTo
// V1.6, 4.7.2016: added copyTo, implemented the missing setPosition()
// V1.5  30.6.2016: Default attack/decay envelope to 0. this is too unexpected! 
// V1.4b, 09.01.2016: Added symmetric attack/decay envelope to MonoSample. Defaults to 128 samples (2ms at 44kHz)
// V1.4a, 4.1.2015: Fixed a bug in mean_abs (result was not initialized)
// V1.3, 23.12.2015: Added normalize and toArray() to monosample. Added clip to AudioAlgo
// V1.2, 18.12.2015: removeHead has a "deleteBuffer" option. defaults to true.
// V1.1, 27.10.2015: addTo returns num copied
// V1.0, 22.6.2015
//

#ifndef __AudioAlgo_h__
#define __AudioAlgo_h__

#include "ofMain.h"
#include "ofConstants.h"
#include <iostream>
#include <vector>
#include <math.h>

#if TARGET_OS_IPHONE
#define USE_ACCELERATE 1
#define USE_BLAS 1
#elif TARGET_OS_MAC
#define USE_ACCELERATE 1
#define USE_BLAS 1
#endif

#ifdef USE_ACCELERATE
#include <Accelerate/Accelerate.h>
#endif

/**
 
 Common utility function when handling audio.
 Hardware acceleration is provided by accelerate/blas if enabled.
 
 **/
class AudioAlgo{
public:
	// absolute value mean
	static float mean_abs( float * buffer, int N ){
		float result = 0;
#ifdef USE_ACCELERATE
		vDSP_meamgv( buffer, 1, &result, N );
#else
		for( int i = N-1; i >= 0; i-- ){
			result += fabsf( buffer[i] );
		}
		result /= N;
#endif
		return result;
	}
	
	// absolute max
	static float max_abs( float * buffer, int N ){
		float result = 0;
#ifdef USE_ACCELERATE
		vDSP_maxmgv( buffer, 1, &result, N );
#else
		for( int i = N-1; i >= 0; i-- ){
			result = MAX( fabsf( buffer[i] ), result );
		}
#endif
		return result;
	}
	
	static void copy( float * destination, int destStride, float * source, int sourceStride, int N ){
#ifdef USE_BLAS
		cblas_scopy(N, source, sourceStride, destination, destStride );
#else
		for( int i = N-1; i >= 0; i-- ){
			destination[i*destStride] = source[i*sourceStride];
		}
#endif
	}
	
	
	static void clip( float * destination, float low, float high, int N ){
#ifdef USE_ACCELERATE
		vDSP_vclip( destination, 1, &low, &high, destination, 1, N );
#else
		for( int i = N-1; i >= 0; i--){
			const float val = destination[i];
			destination[i] = (val<low?low:(val>high?high:val));
		}
#endif
	}
	
	static void scale( float * destination, float factor, int N ){
#ifdef USE_BLAS
		cblas_sscal( N, factor, destination, 1 );
#else
		for( int i = N-1; i >= 0; i-- ){
			destination[i] *= factor;
		}
#endif
	}
	
};


/**
 A utility task for handling many sample based audio use cases.
 
 * Record into the buffer while playing back from it
 * Use different block size for adding/removing samples (better performance when they're the same)
 * Internal locking, so no external locking mechanism is required.
 
 **/
class MonoSample{
	
public:
	
	MonoSample();
	~MonoSample();
	
	void play();
	void playFrom(int position);
	void setPosition( int position );
	void skip( int samples );
	
	// append an array of N floats
	void append( float * buffer, int N );
	// append an array of N floats, using a stride (the array must be N*srcStride elements long)
	void append( float * buffer, int N, int srcStride );
	// remove the top most float buffer from the internal storage (no matter it's size)
	// if deleteBuffer=true then null is returned.
	// if deleteBuffer=false, then you need to manually delete[] the returned buffer.
	float * removeHead(bool deleteBuffer=true);
	// clear out all buffers. reset playback index to -1.
	void clear();
	// remove N samples from internal storage
	void peel( int N);
	// remove the top buffer from the storage and return buffer to the memory (delete manually!). the array size is placed in &numSamples.
	float * peelHead( int &numSamples );
	// return a pointer to the head buffer. you have to know waht you're, else the buffer might get deleted while you're looking at it! returns NULL if not available. doesn't modifiy internal storage.
	float * peekHead( int &numSamples, int bufferNum=0 );
	
	// add the N samples to the *output buffer, skipping with outStride (output must be outStride*N large). this does not remove samples from the internal storage. playback index is increased by N.
	int addTo( float * output, int outStride, int N );
	
	
	// add the N samples to the *output buffer, skipping with outStride (output must be outStride*N large). this does not remove samples from the internal storage. playback index is increased by N.
	// if there are not enough samples, the rest of output will be filled with zeros.
	int copyTo( float * output, int outStride, int N );
	
	// normalizes across all buffers
	void normalize(float max=1.0);
	
	// flattens the entire thing and turns it into a float buffer
	float * toArray();
	
	bool playing;
	int playbackIndex;
	int totalLength;
	bool loop;
	float velocity;
	int attackDecayEnv;
	
	std::vector<float*> bufferData;
	std::vector<int> bufferSizes;
	
	recursive_mutex mut;
private:
	void _skip(int numSamples);
};

#endif /* defined(__AudioAlgo_h__) */
