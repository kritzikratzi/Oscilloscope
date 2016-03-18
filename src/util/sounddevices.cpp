//
//  sounddevices.cpp
//  audioOutputExample
//
//  Created by Hansi on 22.10.14.
//
//

#include "sounddevices.h"

#include "ofRtAudioSoundStream.h"
#include "ofConstants.h"

#include "ofSoundStream.h"
#include "ofMath.h"
#include "ofUtils.h"

using namespace std;

bool getDefaultRtOutputParams( int &deviceId, int &sampleRate, int &bufferSize, int &numBuffers ){
	ofSoundStream stream;
	vector<ofSoundDevice> devices = stream.getDeviceList();
	ofSoundDevice device;
	for (auto it = devices.begin(); it != devices.end(); ++it) {
		device = *it; 
		if (device.isDefaultOutput) {
			break;
		}
	}
	deviceId = device.deviceID;

	vector<unsigned int> &rates = device.sampleRates;
	if( rates.size() == 0 ){
		sampleRate = 44100; // safe guess
	}
	else if( std::find(rates.begin(), rates.end(), 44100) != rates.end()){
		sampleRate = 44100; 
	}
	else{
		sampleRate = *max_element(rates.begin(), rates.end());
	}
	#ifdef _WIN32
		bufferSize = 512;
	#elif __APPLE__
		bufferSize = 512;
	#else
		bufferSize = 1024;
	#endif
	
	numBuffers = 4;

	return true; 
}