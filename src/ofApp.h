#pragma once

#include "ofMain.h"
#include "util/sounddevices.h"
#include "util/ShaderLoader.h"
#include "ofxMightyUI.h"

#include "ui/ConfigView.h"
#include "ui/PlayerOverlay.h"
#include "ui/Playlist.h"
#include "util/Audio.h"
#include "ofxAvAudioPlayer.h"
#include "OsciMesh.h"

class ofApp : public ofBaseApp{

	public:

		void setup();
		void startApplication();
		void stopApplication();
		void update();
		void draw();
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

		void stopMic();
		ofMatrix4x4 getViewMatrix(int i, bool isQuad); 
	
		ofSoundStream soundStream;
		ofSoundStream micStream;

		mui::Root * root;
		ConfigView * configView;
		PlayerOverlay * osciView;
		Playlist * playlist; 
		OsciMesh mesh;
		OsciMesh mesh2;  // for stereoscopic viewing
		ofFbo fbo;
	
		// storage for mic buffers
		MonoSample left;
		MonoSample right;
	
		bool changed;
		bool clearFbos;
		int dropped;
		bool showInfo; 
	
		int exporting;
		int exportFrameNum; 
		string exportDir;
	
		uint64_t lastMouseMoved{0};
		uint64_t lastUpdateTime{0};
		ofVec2f mousePosBeforeHiding;
		string fileToLoad;
		string currentFilename{"Oscilloscope"};
		string nextWindowTitle;
	
		ofVec2f last;
		bool hadWindowFocus = true;
};
