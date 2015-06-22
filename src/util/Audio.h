//
// Audio.h
//
// Initially created by Hansi on 14.06.14.
//
// V1.0, 22.6.2015
//

#ifndef __AudioAlgo_h__
#define __AudioAlgo_h__

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

#include "Poco/Mutex.h"

/**
 
 Common utility function when handling audio.
 Hardware acceleration is provided by accelerate/blas if enabled.
 
 **/
class AudioAlgo{
public:
	// absolute value mean
	static float mean_abs( float * buffer, int N ){
		float result;
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
	void setPosition( int position );
	void skip( int samples );
	
	// append an array of N floats
	void append( float * buffer, int N );
	// append an array of N floats, using a stride (the array must be N*srcStride elements long)
	void append( float * buffer, int N, int srcStride );
	// remove the top most float buffer from the internal storage (no matter it's size)
	void removeHead();
	// clear out all buffers. reset playback index to -1.
	void clear();
	// remove N samples from internal storage
	void peel( int N);
	// remove the top buffer from the storage and return buffer to the memory (delete manually!). the array size is placed in &numSamples.
	float * peelHead( int &numSamples );
	// return a pointer to the head buffer. you have to know waht you're, else the buffer might get deleted while you're looking at it! returns NULL if not available. doesn't modifiy internal storage.
	float * peekHead( int &numSamples, int bufferNum=0 );
	
	// add the N samples to the *output buffer, skipping with outStride (output must be outStride*N large). this does not remove samples from the internal storage. playback index is increased by N.
	void addTo( float * output, int outStride, int N );
	
	bool playing;
	int playbackIndex;
	int totalLength;
	bool loop;
	float velocity;
	
	std::vector<float*> bufferData;
	std::vector<int> bufferSizes;
	
	Poco::Mutex lock;
private:
	void _skip(int numSamples);
};

#endif /* defined(__AudioAlgo_h__) */
