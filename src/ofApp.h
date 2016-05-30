#pragma once

#include "ofMain.h"
#include "util/sounddevices.h"
#include "util/ShaderLoader.h"
#include "ofxMightyUI.h"

#include "ui/ConfigView.h"
#include "ui/OsciView.h"
#include "util/Audio.h"
#include "ofxAvAudioPlayer.h"

#include "ofxTCPServer.h"
#include "ofThread.h"

class ofApp;
class Fetcher : public ofThread{
public:
	Fetcher( ofApp * app );
	void threadedFunction();
	
	ofApp * app;
};


class ofApp : public ofBaseApp{

	public:

		void setup();
		void startApplication();
		void stopApplication();
		void update();
		void update( ofMesh &shapeMesh, MonoSample &left, MonoSample &right, bool & changed, ofPoint & last, int index );
		void draw();
		void draw(ofFbo & fbo, ofMesh & shapeMesh, bool & changed );
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
		ofMesh shapeMesh1;
		ofMesh shapeMesh2;
		ofFbo fbo1;
		ofFbo fbo2;
		ofShader shader;
		ShaderLoader shaderLoader;
	
		MonoSample left1;
		MonoSample right1;
		MonoSample left2;
		MonoSample right2;
	
		bool changed1;
		bool changed2;
		bool clearFbos;
		int dropped; 
		bool showInfo; 
	
		int exporting;
		int exportFrameNum; 
		string exportDir;
	
	
		unsigned long long lastMouseMoved;
		string fileToLoad;
	
		ofPoint last1;
		ofPoint last2;
	
		ofxTCPServer * TCP;
		Fetcher * fetcher;

};
