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

vector<RtAudio::DeviceInfo> listRtSoundDevices(){
	vector<RtAudio::DeviceInfo> infos;
	ofPtr<RtAudio> audioTemp;
	try {
		audioTemp = ofPtr<RtAudio>(new RtAudio());
	} catch (std::exception &error) {
		return infos;
	}
	
 	int devices = audioTemp->getDeviceCount();
	RtAudio::DeviceInfo info;
	
	for (int i=0; i< devices; i++) {
		try {
			info = audioTemp->getDeviceInfo(i);
			infos.push_back( info );
		} catch (std::exception &error) {
			break;
		}
	}
	
	return infos; 
}

bool getDefaultRtOutputParams( int &deviceId, int &sampleRate, int &bufferSize, int &numBuffers ){
	ofPtr<RtAudio> audioTemp;
	try {
		audioTemp = ofPtr<RtAudio>(new RtAudio());
	} catch (std::exception ex) {
		return false;
	}
	
	deviceId = audioTemp->getDefaultOutputDevice();
	RtAudio::DeviceInfo info = audioTemp->getDeviceInfo(deviceId);
	
	vector<unsigned int> &rates = info.sampleRates;
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
		bufferSize = 1024;
	#elif __APPLE__
		bufferSize = 512;
	#else
		bufferSize = 1024;
	#endif
	
	numBuffers = 4;
}