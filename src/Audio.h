//
//  Audio.h
//  audioInputExample
//
//  Created by Hansi on 14.06.14.
//
//

#ifndef __audioInputExample__Audio__
#define __audioInputExample__Audio__

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

class Audio{
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
				result = fmaxf( fabsf( buffer[i] ), result );
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


class MonoSample{

public:
	
	MonoSample();
	~MonoSample(); 
	
	void play();
	void setPosition( int position );
	
	void append( float * buffer, int N );
	void append( float * buffer, int N, int srcStride );
	void removeHead();
	void peel( int N);
	
	void addTo( float * output, int outStride, int N );
	
	int playbackIndex;
	int totalLength;
	bool loop; 
	float velocity;
	
	std::vector<float*> bufferData;
	std::vector<int> bufferSizes;
	
	Poco::Mutex lock; 
};

#endif /* defined(__audioInputExample__Audio__) */
