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


vector<RtAudio::DeviceInfo> listRtSoundDevices(){
	vector<RtAudio::DeviceInfo> infos;
	ofPtr<RtAudio> audioTemp;
	try {
		audioTemp = ofPtr<RtAudio>(new RtAudio());
	} catch (RtError &error) {
		error.printMessage();
		return infos;
	}
	
 	int devices = audioTemp->getDeviceCount();
	RtAudio::DeviceInfo info;
	
	for (int i=0; i< devices; i++) {
		try {
			info = audioTemp->getDeviceInfo(i);
			infos.push_back( info );
		} catch (RtError &error) {
			error.printMessage();
			break;
		}
	}
	
	return infos; 
}