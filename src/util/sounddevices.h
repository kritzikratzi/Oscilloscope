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



bool getDefaultRtOutputParams( int &deviceId, int &sampleRate, int &bufferSize, int &numBuffers );