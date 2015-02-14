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
	playbackIndex = -1; 
	totalLength = 0; 
	velocity = 1;
	loop = false;
}


MonoSample::~MonoSample(){
	//for( int i = 0; i < bufferData.size(); i++ ){
	//	delete[] bufferData[i];
	//}
	//bufferData.clear();
	while(!bufferData.empty()) {
        delete[] bufferData.back();
        bufferData.pop_back();
		bufferSizes.pop_back();
    }
}

void MonoSample::play(){
	lock.lock();
	playbackIndex = 0;
	lock.unlock();
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
	Audio::copy(data, 1, buffer, srcStride, N);
	lock.lock();
	bufferData.push_back( data );
	bufferSizes.push_back( N );
	totalLength += N;
	lock.unlock();
}

void MonoSample::removeHead(){
	lock.lock();
	if( bufferData.size() > 0 ){
		float * first = bufferData[0];
		totalLength -= bufferSizes[0];
		playbackIndex -= bufferSizes[0];
		bufferData.erase(bufferData.begin());
		bufferSizes.erase(bufferSizes.begin());
		
		delete[] first;
	}
	lock.unlock();
}

void MonoSample::peel(int N){
	int NN = N;
	lock.lock();
	N = MIN(N,totalLength); 
	while( N > 0 && totalLength > 0 ){
		if( bufferSizes[0] == N ){
			removeHead();
			lock.unlock();
			return;
		}
		else if( bufferSizes[0] < N ){
			removeHead();
			// delete another
		}
		else{
			// todo: this has some problems it seems. 
			int remaining = bufferSizes[0]-N;
			float * buffer = new float[remaining];
			memcpy(buffer, bufferData[0], N*sizeof(float));
			float * old = bufferData[0];
			bufferData[0] = buffer;
			bufferSizes[0] = N;
			delete[] old;
			N = remaining;
		}
	}
	playbackIndex = MAX(0,playbackIndex-N);
	totalLength -= N;
	lock.unlock();
}

void MonoSample::addTo(float *output, int outStride, int N ){
	if( playbackIndex == -1 ){
		return;
	}
	
	lock.lock();
	//TODO: make this faster!
	// find the right index...
	int copied = 0;
	
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
					output[outStride*copied] += source[j]*velocity;
					copied ++;
				}
				
				playbackIndex += copyN;
				if( copied >= N ){
					// done for today!
					lock.unlock();
					return;
				}
			}
			len += sourceN;
		}
		
		if( playbackIndex >= len ){
			if( loop == true && totalLength > 0 ){
				playbackIndex = 0;
			}
			else{
				playbackIndex = -1;
				lock.unlock();
				return;
			}
		}
	}
	lock.unlock();
}