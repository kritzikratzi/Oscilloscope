//
//  globals.cpp
//  Oscilloscope
//
//  Created by Hansi on 26.07.15.
//
//

#include "globals.h"
#include "util/WickedLasers.h"
#include "util/miniaudio.h"

Globals Globals::instance; 

void Globals::init() {
	laserPtr = make_shared<WickedLasers>();
	context = new ma_context();
	if (ma_context_init(NULL, 0, NULL, context) != MA_SUCCESS) {
		ofSystemAlertDialog("Failed to initialize audio context :(");
		
	}
}

string ofxFormatTime(double seconds) {
	int total = seconds; 
	int s = total % 60; 
	
	total = total / 60; 
	int m = total % 60; 

	total = total / 60; 
	int h = total; 

	return ofToString(h, 2, '0') + ":" + ofToString(m, 2, '0') + ":" + ofToString(s, 2, '0'); 
}


void Globals::loadFromFile(string settingsFile) {
	ofxIniSettings settings = ofxIniSettings(settingsFile);
	out_requested.bufferSize = settings.get("out_bufferSize", out_requested.bufferSize);
	out_requested.sampleRate = settings.get("out_sampleRate", out_requested.sampleRate);
	out_requested.name = settings.get("out_deviceName", out_requested.name);
	in_requested.bufferSize = settings.get("in_bufferSize", in_requested.sampleRate);
	in_requested.sampleRate = settings.get("in_sampleRate", in_requested.sampleRate);
	in_requested.name = settings.get("in_deviceName", in_requested.name);

	scale = settings.get("scale", scale);
	flipXY = settings.get("flipXY", flipXY);
	zModulation = settings.get("zModulation", zModulation);
	invertX = settings.get("invertX", invertX);
	invertY = settings.get("invertY", invertY);
	outputVolume = settings.get("outputVolume", outputVolume);
	inputVolume = settings.get("inputVolume", inputVolume);
	strokeWeight = settings.get("strokeWeight", strokeWeight);
	//timeStretch = settings.get( "timeStretch", timeStretch ); // never load timestretch!
	blur = settings.get("blur", blur);
	numPts = settings.get("numPts", numPts);
	hue = settings.get("hue", hue);
	intensity = settings.get("intensity", intensity);
	afterglow = settings.get("afterglow", afterglow);
	analogMode = settings.get("analogMode", analogMode);
	exportFrameRate = settings.get("exportFrameRate", exportFrameRate);
	exportWidth = settings.get("exportWidth", exportWidth);
	exportHeight = settings.get("exportHeight", exportHeight);
	exportFormat = (ExportFormat)settings.get("exportFormat", (int)exportFormat);
	exportSampleRate = settings.get("exportSampleRate", (int)exportSampleRate);

	laserOffsetX = settings.get("laserOffsetX", laserOffsetX);
	laserOffsetY = settings.get("laserOffsetY", laserOffsetY);
	laserKeystoneX = settings.get("laserKeystoneX", laserKeystoneX);
	laserKeystoneY = settings.get("laserKeystoneY", laserKeystoneY);

	//secondsBeforeHidingMenu = settings.get( "secondsBeforeHidingMenu", secondsBeforeHidingMenu );
}


void Globals::saveToFile(string settingsFile) {
	if (!ofFile(settingsFile, ofFile::Reference).exists()) {
		ofFile file(settingsFile, ofFile::WriteOnly);
		file << endl;
		file.close();
	}

	ofxIniSettings settings = ofxIniSettings(settingsFile);

	//settings.set("out_bufferSize", out_requested.bufferSize);
	//settings.set("out_sampleRate", out_requested.sampleRate);
	settings.set("out_deviceName", out_requested.name);
	//settings.set("in_bufferSize", in_requested.bufferSize);
	//settings.set("in_sampleRate", in_requested.sampleRate);
	settings.set("in_deviceName", in_requested.name);
	settings.set("scale", scale);
	settings.set("flipXY", flipXY);
	settings.set("zModulation", zModulation);
	settings.set("invertX", invertX);
	settings.set("invertY", invertY);
	settings.set("outputVolume", outputVolume);
	settings.set("inputVolume", inputVolume);
	settings.set("strokeWeight", strokeWeight);
	// settings.set( "timeStretch", timeStretch ); // never save timestretch!
	settings.set("blur", blur);
	settings.set("numPts", numPts);
	settings.set("hue", hue);
	settings.set("intensity", intensity);
	settings.set("afterglow", afterglow);
	settings.set("analogMode", analogMode);
	settings.set("exportFrameRate", exportFrameRate);
	settings.set("exportWidth", exportWidth);
	settings.set("exportHeight", exportHeight);
	settings.set("exportFormat", (int)exportFormat);
	settings.set("exportSampleRate", (int)exportSampleRate);

	settings.set("laserOffsetX", laserOffsetX);
	settings.set("laserOffsetY", laserOffsetY);
	settings.set("laserKeystoneX", laserKeystoneX);
	settings.set("laserKeystoneY", laserKeystoneY);

	// settings.set( "secondsBeforeHidingMenu", secondsBeforeHidingMenu );
}
