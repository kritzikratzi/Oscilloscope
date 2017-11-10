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
	lock.lock();
	playing = true;
	lock.unlock();
}

void MonoSample::playFrom(int position){
	lock.lock();
	playing = true;
	playbackIndex = 0;
	lock.unlock();
}

void MonoSample::setPosition( int position ){
	lock.lock();
	playbackIndex = position;
	lock.unlock(); 
}


void MonoSample::skip( int numSamples ){
	lock.lock();
	_skip(numSamples);
	lock.unlock();
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
	lock.lock();
	bufferData.push_back( data );
	bufferSizes.push_back( N );
	totalLength += N;
	lock.unlock();
}

void MonoSample::append(float * buffer, int N, int srcStride){
	float * data = new float[N];
	AudioAlgo::copy(data, 1, buffer, srcStride, N);
	lock.lock();
	bufferData.push_back( data );
	bufferSizes.push_back( N );
	totalLength += N;
	lock.unlock();
}

float * MonoSample::removeHead(bool deleteBuffer){
	lock.lock();
	if( bufferData.size() > 0 ){
		float * first = bufferData[0];
		totalLength -= bufferSizes[0];
		_skip(-bufferSizes[0]);
		bufferData.erase(bufferData.begin());
		bufferSizes.erase(bufferSizes.begin());
		
		lock.unlock();
		
		if( deleteBuffer){
			delete[] first;
			return NULL;
		}
		else{
			return first;
		}
	}
	lock.unlock();
	return NULL;
}

void MonoSample::clear(){
	lock.lock();
	while( bufferData.size() > 0 ){
		removeHead();
	}
	playbackIndex = 0;
	playing = false;
	lock.unlock();
}

void MonoSample::peel(int N){
	lock.lock();
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
	
	lock.unlock();
}

float * MonoSample::peelHead( int &numSamples ){
	lock.lock();
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
	
	lock.unlock();
	return result;
}

float * MonoSample::peekHead( int &numSamples, int bufferNum ){
	lock.lock();
	float * result = NULL;
	if( bufferSizes.size() > bufferNum ){
		numSamples = bufferSizes[bufferNum];
		result = bufferData[bufferNum];
	}
	else{
		numSamples = 0;
	}
	
	lock.unlock();
	
	return result;
}



int MonoSample::addTo(float *output, int outStride, int N ){
	if( !playing ){
		return 0;
	}
	
	lock.lock();
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
					lock.unlock();
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
				lock.unlock();
				return copied;
			}
		}
	}
	lock.unlock();
	return copied;
}



int MonoSample::copyTo(float *output, int outStride, int N ){
	int copied = 0;
	int envEnd = totalLength - attackDecayEnv - 1; // we don't take care of buffers shorter than 2*attack_decay_env!
	
	if( !playing ){
		goto zeroPad;
	}
	
	lock.lock();
	
	
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
					lock.unlock();
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
				lock.unlock();
				return copied;
			}
		}
	}
	lock.unlock();
	
zeroPad:
	for( int i = copied; i < N; i++ ){
		output[i*outStride] = 0;
	}
	return copied;
}

void MonoSample::normalize(float max){
	lock.lock();
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
	lock.unlock();
}

float * MonoSample::toArray(){
	lock.lock();
	float * result = new float[totalLength];
	int pos = 0;
	for( int i = 0; i < bufferData.size(); i++ ){
		memcpy(result+pos, bufferData[i], bufferSizes[i]*sizeof(float));
		pos += bufferSizes[i];
	}
	lock.unlock();
	
	return result;
}