#pragma once

#include "ofMain.h"
#include "util/sounddevices.h"
#include "util/ShaderLoader.h"
#include "ofxMightyUI.h"

#include "ui/ConfigView.h"
#include "ui/OsciView.h"
#include "util/Audio.h"
#include "ofxAvAudioPlayer.h"

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

		ofMatrix4x4 getViewMatrix();
	
		ofSoundStream soundStream;
		ofSoundStream micStream;

		mui::Root * root;
		ConfigView * configView;
		OsciView * osciView;
		ofPath path;
		ofMesh shapeMesh;
		ofFbo fbo;
		ofShader shader;
		ShaderLoader shaderLoader;
	
		MonoSample left;
		MonoSample right;
	
		bool changed;
		bool clearFbos;
		int dropped; 
		bool showInfo; 
	
		int exporting;
		int exportFrameNum; 
		string exportDir;
	
	
		unsigned long long lastMouseMoved;
		string fileToLoad;
	
		ofVec2f last;
};
