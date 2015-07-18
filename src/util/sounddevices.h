//
//  sounddevices.h
//  audioOutputExample
//
//  Created by Hansi on 22.10.14.
//
//

#pragma once

#include "ofMain.h"
#include "RtAudio.h"



vector<RtAudio::DeviceInfo> listRtSoundDevices();
bool getDefaultRtOutputParams( int &deviceId, int &sampleRate, int &bufferSize, int &numBuffers );