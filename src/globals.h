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
class WickedLasers;
struct ma_context; 

#define globals (Globals::instance)
class Globals{
public:
	Globals(){};
	void init(); 
	
	struct AudioConfig {
		string name; 
		int bufferSize; 
		int sampleRate; 
	};

	ma_context * context;
	
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
	int analogMode{0}; // 0=digital, 1=analog, 2=superanalog
	
	int numPts{20}; // 1...+inf?
	float hue{50}; // 0...360
	
	float outputVolume{1};
	float inputVolume{1};
	
	int exportWidth{1920};
	int exportHeight{1080};
	int exportFrameRate{60};
	ExportFormat exportFormat{ExportFormat::IMAGE_SEQUENCE_PNG};
	
	float secondsBeforeHidingMenu{0.5};
	
	bool alwaysOnTop{false};

	// laser stuff. none of this is ever stored!
	shared_ptr<WickedLasers> laserPtr{nullptr}; 
	bool laserConnected{ false };
	float laserSize{ 0.5 };
	float laserIntensity{ 0.0 };
	float laserKeystoneX{ 0.0 };
	float laserKeystoneY{ 0.0 };
	float laserOffsetX{ 0.0 };
	float laserOffsetY{ 0.0 };

	void loadFromFile(string settingsFile = ofxToReadWriteableDataPath("settings.txt"));
	void saveToFile(string settingsFile = ofxToReadWriteableDataPath("settings.txt"));
	
	// runtime variables (not saved)
	OsciAvAudioPlayer player;
	size_t currentlyPlayingItem = 0;

	// the singleton thing
	static Globals instance;
};


#endif /* defined(__Oscilloscope__globals__) */
