#pragma once

#include "util/miniaudio.h"

#include "ofMain.h"
#include "util/ShaderLoader.h"
#include "ofxMightyUI.h"

#include "ui/ConfigView.h"
#include "ui/PlayerOverlay.h"
#include "ui/Playlist.h"
#include "util/Audio.h"
#include "ofxAvAudioPlayer.h"
#include "OsciMesh.h"
#include "util/TexShare.h"

class ExportScreen;
enum class ExportFormat;


class ofApp : public ofBaseApp{

	public:

		void setup();
		void startApplication();
		void stopApplication();
		void update();
		void draw();
		void drawAbove();
		void exit();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void audioIn(float * input, int bufferSize, int nChannels);
		void audioOut( float * output, int bufferSize, int nChannels ); 

		void beginExport(const ofFile & file);
		void playlistItemEnded();
		void updateSampleRate(); 

		void startMic(); 
		void stopMic();
		ofMatrix4x4 getViewMatrix(int i, bool isQuad); 

		void urlResponse(ofHttpResponse & response);


		mui::Root * root;
		ConfigView * configView;
		ExportScreen * exportScreen;
	
		PlayerOverlay * playerOverlay;
		Playlist * playlist;
		bool playlistEnable = false;
		OsciMesh mesh;
		OsciMesh mesh2;  // for stereoscopic viewing
		
		ofFbo fbo;
		TexShare texSender;
	
		// storage for mic buffers
		int micChannels = 2;
		MonoSample micLeft;
		MonoSample micRight;
		MonoSample micZMod;

		ma_device_config playDeviceConfig;
		ma_device playDevice;

		ma_device_config micDeviceConfig;
		ma_device micDevice;

		bool changed;
		bool clearFbos;
		int dropped;
		bool showInfo; 
	
		int exporting;
		int exportFrameNum;
		ExportFormat exportFormat; 
		string exportDir;
	
		uint64_t lastMouseMoved{0};
		uint64_t lastUpdateTime{0};
		ofVec2f mousePosBeforeHiding;
		bool forceHidden = false; 
		string fileToLoad;
		string currentFilename{"Oscilloscope"};
		string nextWindowTitle;
	
		ofVec2f last;
		bool hadWindowFocus = true;
		bool applicationRunning = false;
		bool initComplete = false; 
		ofxFontStashStyle tooltipStyle;
	
		mutex mainThreadMutex;
		queue<function<void()>> mainThreadTasks;
};
