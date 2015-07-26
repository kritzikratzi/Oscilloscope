//
//  globals.h
//  Oscilloscope
//
//  Created by Hansi on 26.07.15.
//
//

#ifndef __Oscilloscope__globals__
#define __Oscilloscope__globals__

#include <stdio.h>
#include "ofxAvAudioPlayer.h"
#include "ofxIniSettings.h"

#define globals (Globals::instance)
class Globals{
public:
	Globals() :  sampleRate(44100), bufferSize(512), numBuffers(4), deviceId(0),scale(1.0),flipXY(false),invertX(false),invertY(false),autoDetect(true), outputVolume(1), inputVolume(1){
	}
	
	// audio settings
	bool autoDetect;
	int sampleRate;
	int bufferSize;
	int numBuffers;
	int deviceId;
	
	// display settings
	float scale;
	bool invertX;
	bool invertY;
	bool flipXY;
	
	float outputVolume;
	float inputVolume;
	
	
	void loadFromFile( string settingsFile = ofToDataPath("settings.txt") ){
		ofxIniSettings settings = ofxIniSettings(settingsFile);
		bufferSize = settings.get( "bufferSize", bufferSize );
		sampleRate = settings.get("sampleRate",  sampleRate );
		numBuffers = settings.get( "numBuffers", numBuffers );
		deviceId = settings.get( "deviceId", deviceId );
		scale = settings.get( "scale", scale );
		flipXY = settings.get( "flipXY", flipXY );
		invertX = settings.get( "invertX", invertX );
		invertY = settings.get( "invertY", invertY );
		autoDetect = settings.get( "autoDetect", autoDetect );
		outputVolume = settings.get( "outputVolume", outputVolume );
		inputVolume = settings.get( "inputVolume", inputVolume );
	}
	
	
	void saveToFile( string settingsFile = ofToDataPath("settings.txt") ){
		ofxIniSettings settings = ofxIniSettings(settingsFile);
		
		settings.set( "bufferSize", bufferSize );
		settings.set( "sampleRate", sampleRate );
		settings.set( "numBuffers", numBuffers );
		settings.set( "deviceId", deviceId );
		settings.set( "scale", scale );
		settings.set( "flipXY", flipXY );
		settings.set( "invertX", invertX );
		settings.set( "invertY", invertY );
		settings.set( "autoDetect", autoDetect );
		settings.set( "outputVolume", outputVolume );
		settings.set( "inputVolume", inputVolume );
	}
	
	
	
	// runtime variables (not saved)
	ofxAvAudioPlayer player;
	
	
	// the singleton thing
	static Globals instance;
};


#endif /* defined(__Oscilloscope__globals__) */
