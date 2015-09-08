#pragma once

#include "ofMain.h"
#include "util/sounddevices.h"
#include "util/ShaderLoader.h"
#include "MUI.h" 

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
	
		ofSoundStream soundStream;

		mui::Root * root;
		ConfigView * configView;
		OsciView * osciView;
		ofPath path;
		ofMesh shapeMesh;
		ofImage dotImage;
		ofFbo fbo, fbb;
		ofShader shader;
		ShaderLoader shaderLoader;
	
		ofShader blur;
	
		MonoSample left;
		MonoSample right;
	
		bool changed;
		bool clearFbos; 
	
	unsigned long long lastMouseMoved; 
};
