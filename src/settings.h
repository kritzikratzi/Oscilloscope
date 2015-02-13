//
//  settings.h
//  audioOutputExample
//
//  Created by Hansi on 22.10.14.
//
//

#pragma once

#include "ofMain.h" 
#include "ofxIniSettings.h"


class Settings{
public: 
	int sampleRate;
	int bufferSize;
	int numBuffers;
	int deviceId;
	float scale;
	bool flipXY;
	bool invertX;
	bool invertY;
	int port;
	
	Settings() :  sampleRate(44100), bufferSize(512), numBuffers(4), deviceId(0),scale(1.0),flipXY(false),invertX(false),invertY(false), port(1234){
	}
	
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
		port = settings.get( "port", port );
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
		settings.set( "port", port );
	}
};