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
#include "OsciAvAudioPlayer.h"
#include "ofxIniSettings.h"

extern string ofxToReadWriteableDataPath( string filename );
extern string ofxToReadonlyDataPath( string filename );
extern void setWindowRepresentedFilename( string filename );

string ofxFormatTime(double seconds); 

enum class ExportFormat{H264=1,IMAGE_SEQUENCE_PNG=2,IMAGE_SEQUENCE_TIFF=3};

#define globals (Globals::instance)
class Globals{
public:
	Globals(){}
	
	struct AudioConfig {
		string name; 
		int bufferSize; 
		int sampleRate; 
	};

	// audio settings
	AudioConfig out_requested{ "",0, 0 };
	AudioConfig out_actual{ "",512, 44100 };
	
	bool micActive{ false };
	AudioConfig in_requested{ "",0, 0 };
	AudioConfig in_actual{ "",512, 44100 };

	// display settings
	float scale{1.0};
	bool invertX{false};
	bool invertY{false};
	bool flipXY{false};
	bool zModulation{true}; 
	
	float strokeWeight{10}; // 1...20
	float timeStretch{1}; // 0.1-2.0
	float blur{30}; // 0...255
	float intensity{0.4f}; // 0...1
	float afterglow{0.5f}; // 0...1
	
	int numPts{20}; // 1...+inf?
	
	float outputVolume{1};
	float inputVolume{1};
	
	int exportWidth{1920};
	int exportHeight{1080};
	int exportFrameRate{60};
	ExportFormat exportFormat{ExportFormat::IMAGE_SEQUENCE_PNG};
	
	float secondsBeforeHidingMenu{0.5};
	
	bool alwaysOnTop{false};
	
	void loadFromFile( string settingsFile = ofxToReadWriteableDataPath("settings.txt") ){
		ofxIniSettings settings = ofxIniSettings(settingsFile);
		out_requested.bufferSize = settings.get("out_bufferSize", out_requested.bufferSize);
		out_requested.sampleRate = settings.get("out_sampleRate", out_requested.sampleRate);
		out_requested.name = settings.get("out_deviceName", out_requested.name);
		in_requested.bufferSize = settings.get("in_bufferSize", in_requested.sampleRate);
		in_requested.sampleRate = settings.get("in_sampleRate", in_requested.sampleRate);
		in_requested.name = settings.get("in_deviceName", in_requested.name);

		scale = settings.get( "scale", scale );
		flipXY = settings.get( "flipXY", flipXY );
		zModulation = settings.get( "zModulation", zModulation );
		invertX = settings.get( "invertX", invertX );
		invertY = settings.get( "invertY", invertY );
		outputVolume = settings.get( "outputVolume", outputVolume );
		inputVolume = settings.get( "inputVolume", inputVolume );
		strokeWeight = settings.get( "strokeWeight", strokeWeight );
		//timeStretch = settings.get( "timeStretch", timeStretch ); // never load timestretch!
		blur = settings.get( "blur", blur );
		numPts = settings.get( "numPts", numPts );
		intensity = settings.get( "intensity", intensity );
		afterglow = settings.get( "afterglow", afterglow );
		exportFrameRate = settings.get( "exportFrameRate", exportFrameRate );
		exportWidth = settings.get( "exportWidth", exportWidth );
		exportHeight = settings.get( "exportHeight", exportHeight );
		exportFormat = (ExportFormat)settings.get("exportFormat", (int)exportFormat);
		//secondsBeforeHidingMenu = settings.get( "secondsBeforeHidingMenu", secondsBeforeHidingMenu );
	}
	
	
	void saveToFile( string settingsFile = ofxToReadWriteableDataPath("settings.txt") ){
		if(!ofFile(settingsFile, ofFile::Reference).exists()){
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
		settings.set( "scale", scale );
		settings.set( "flipXY", flipXY );
		settings.set( "zModulation", zModulation );
		settings.set( "invertX", invertX );
		settings.set( "invertY", invertY );
		settings.set( "outputVolume", outputVolume );
		settings.set( "inputVolume", inputVolume );
		settings.set( "strokeWeight", strokeWeight );
		// settings.set( "timeStretch", timeStretch ); // never save timestretch!
		settings.set( "blur", blur );
		settings.set( "numPts", numPts );
		settings.set( "intensity", intensity );
		settings.set( "afterglow", afterglow );
		settings.set( "exportFrameRate", exportFrameRate );
		settings.set( "exportWidth", exportWidth );
		settings.set( "exportHeight", exportHeight );
		settings.set( "exportFormat", (int)exportFormat );
		// settings.set( "secondsBeforeHidingMenu", secondsBeforeHidingMenu );
	}
	
	
	
	// runtime variables (not saved)
	OsciAvAudioPlayer player;
	size_t currentlyPlayingItem = 0;

	// the singleton thing
	static Globals instance;
};


#endif /* defined(__Oscilloscope__globals__) */
