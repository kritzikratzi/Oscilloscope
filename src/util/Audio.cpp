//
//  Audio.cpp
//  audioInputExample
//
//  Created by Hansi on 14.06.14.
//
//

#include "Audio.h"

using namespace std;

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#endif

MonoSample::MonoSample(){
	attackDecayEnv = 0;
	playbackIndex = 0;
	totalLength = 0;
	velocity = 1;
	loop = false;
}


MonoSample::~MonoSample(){
	while(!bufferData.empty()) {
		delete[] bufferData.back();
		bufferData.pop_back();
		bufferSizes.pop_back();
	}
}

void MonoSample::play(){
	lock_guard<recursive_mutex> lock(mut);
	playing = true;
}

void MonoSample::playFrom(int position){
    lock_guard<recursive_mutex> lock(mut);
	playing = true;
	playbackIndex = 0;
}

void MonoSample::setPosition( int position ){
    lock_guard<recursive_mutex> lock(mut);
	playbackIndex = position;
}


void MonoSample::skip( int numSamples ){
    lock_guard<recursive_mutex> lock(mut);
	_skip(numSamples);
}

void MonoSample::_skip( int numSamples ){
	if( totalLength == 0 ){
		playbackIndex = 0;
	}
	else{
		playbackIndex = (playbackIndex+numSamples)%totalLength;
		if( playbackIndex < 0 ) playbackIndex += totalLength;
	}
}

void MonoSample::append(float * buffer, int N){
	float * data = new float[N];
	memcpy( data, buffer, sizeof(float)*N );
    lock_guard<recursive_mutex> lock(mut);
	bufferData.push_back( data );
	bufferSizes.push_back( N );
	totalLength += N;
}

void MonoSample::append(float * buffer, int N, int srcStride){
	float * data = new float[N];
	AudioAlgo::copy(data, 1, buffer, srcStride, N);
    lock_guard<recursive_mutex> lock(mut);
	bufferData.push_back( data );
	bufferSizes.push_back( N );
	totalLength += N;
}

float * MonoSample::removeHead(bool deleteBuffer){
    lock_guard<recursive_mutex> lock(mut);
	if( bufferData.size() > 0 ){
		float * first = bufferData[0];
		totalLength -= bufferSizes[0];
		_skip(-bufferSizes[0]);
		bufferData.erase(bufferData.begin());
		bufferSizes.erase(bufferSizes.begin());
		
		if( deleteBuffer){
			delete[] first;
			return NULL;
		}
		else{
			return first;
		}
	}
	return NULL;
}

void MonoSample::clear(){
    lock_guard<recursive_mutex> lock(mut);
	while( bufferData.size() > 0 ){
		removeHead();
	}
	playbackIndex = 0;
	playing = false;
}

void MonoSample::peel(int N){
    lock_guard<recursive_mutex> lock(mut);
	N = MIN(N,totalLength);
	int NN = N;
	while( N > 0 && totalLength > 0 ){
		if( bufferSizes[0] <= N ){
			N -= bufferSizes[0];
			removeHead();
		}
		else{
			// todo: this has some problems it seems.
			int remaining = bufferSizes[0]-N;
			float * buffer = new float[remaining];
			memcpy(buffer, &bufferData[0][N], remaining*sizeof(float));
			float * old = bufferData[0];
			bufferData[0] = buffer;
			bufferSizes[0] = remaining;
			delete[] old;
			
			_skip(-N);
			totalLength -= N;
			break;
		}
	}
}

float * MonoSample::peelHead( int &numSamples ){
    lock_guard<recursive_mutex> lock(mut);
	float * result = NULL;
	
	if( bufferSizes.size() > 0 ){
		numSamples = bufferSizes[0];
		result = bufferData[0];
		bufferData.erase(bufferData.begin());
		bufferSizes.erase(bufferSizes.begin());
		_skip(-numSamples);
	}
	else{
		numSamples = 0;
	}
	
	return result;
}

float * MonoSample::peekHead( int &numSamples, int bufferNum ){
    lock_guard<recursive_mutex> lock(mut);
	float * result = NULL;
	if( bufferSizes.size() > bufferNum ){
		numSamples = bufferSizes[bufferNum];
		result = bufferData[bufferNum];
	}
	else{
		numSamples = 0;
	}
	
	return result;
}



int MonoSample::addTo(float *output, int outStride, int N ){
	if( !playing ){
		return 0;
	}
	
    lock_guard<recursive_mutex> lock(mut);
	//TODO: make this faster!
	// find the right index...
	int copied = 0;
	
	int envEnd = totalLength - attackDecayEnv - 1; // we don't take care of buffers shorter than 2*attack_decay_env!
	
	while( copied < N ){
		// find the right buffer...
		int len = 0;
		for( int i = 0; i < bufferData.size(); i++ ){
			int sourceN = bufferSizes[i];
			
			if( len + sourceN > playbackIndex ){
				// good, we have something to copy ...
				float * source = bufferData[i];
				int  sourceStart = playbackIndex - len;
				int copyN = MIN( sourceN, N - copied );
				
				for( int j = sourceStart; j < copyN; j++ ){
					// this could be more optimal? not sure!
					float env =
					playbackIndex>envEnd?ofMap(playbackIndex-envEnd, 0, attackDecayEnv, 1, 0):
					(playbackIndex<attackDecayEnv?ofMap(playbackIndex,0,attackDecayEnv,0,1):1);
					
					output[outStride*copied] += source[j]*velocity*env;
					copied ++;
					playbackIndex ++;
				}
				
				if( copied >= N ){
					// done for today!
					return copied;
				}
			}
			len += sourceN;
		}
		
		if( playbackIndex >= len ){
			if( loop == true && totalLength > 0 ){
				playbackIndex = 0;
			}
			else{
				playbackIndex = 0;
				playing = false;
				return copied;
			}
		}
	}
	return copied;
}



int MonoSample::copyTo(float *output, int outStride, int N ){
	int copied = 0;
	int envEnd = totalLength - attackDecayEnv - 1; // we don't take care of buffers shorter than 2*attack_decay_env!
	
    lock_guard<recursive_mutex> lock(mut);
	if( !playing ){
		goto zeroPad;
	}
	

	
	while( copied < N ){
		// find the right buffer...
		int len = 0;
		for( int i = 0; i < bufferData.size(); i++ ){
			int sourceN = bufferSizes[i];
			
			if( len + sourceN > playbackIndex ){
				// good, we have something to copy ...
				float * source = bufferData[i];
				int  sourceStart = playbackIndex - len;
				int copyN = MIN( sourceN, N - copied );
				
				for( int j = sourceStart; j < copyN; j++ ){
					// this could be more optimal? not sure!
					float env =
					playbackIndex>envEnd?ofMap(playbackIndex-envEnd, 0, attackDecayEnv, 1, 0):
					(playbackIndex<attackDecayEnv?ofMap(playbackIndex,0,attackDecayEnv,0,1):1);
					
					output[outStride*copied] = source[j]*velocity*env;
					copied ++;
					playbackIndex ++;
				}
				
				if( copied >= N ){
					// done for today!
					return copied;
				}
			}
			len += sourceN;
		}
		
		if( playbackIndex >= len ){
			if( loop == true && totalLength > 0 ){
				playbackIndex = 0;
			}
			else{
				playbackIndex = 0;
				playing = false;
				return copied;
			}
		}
	}
	
zeroPad:
	for( int i = copied; i < N; i++ ){
		output[i*outStride] = 0;
	}
	return copied;
}

void MonoSample::normalize(float max){
    lock_guard<recursive_mutex> lock(mut);
	float maxFound = 0;
	for( int i = 0; i < bufferSizes.size(); i++ ){
		float val = AudioAlgo::max_abs(bufferData[i], bufferSizes[i] );
		if( val > maxFound ) maxFound = val;
	}
	if( maxFound > 0 ){
		float factor = max/maxFound;
		for( int i = 0; i < bufferSizes.size(); i++ ){
			AudioAlgo::scale(bufferData[i], factor, bufferSizes[i] );
		}
	}
}

float * MonoSample::toArray(){
    lock_guard<recursive_mutex> lock(mut);
	float * result = new float[totalLength];
	int pos = 0;
	for( int i = 0; i < bufferData.size(); i++ ){
		memcpy(result+pos, bufferData[i], bufferSizes[i]*sizeof(float));
		pos += bufferSizes[i];
	}
	
	return result;
}
