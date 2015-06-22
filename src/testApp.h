#pragma once

#include "ofMain.h"
#include "sounddevices.h"
#include "MUI.h" 
#include "ConfigView.h"
#include "OsciView.h"
#include "Audio.h"

class testApp : public ofBaseApp{

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
		
		ofSoundStream soundStream;

		mui::Root * root;
		ConfigView * configView;
		OsciView * osciView;
		ofPath path;
		ofMesh shapeMesh;
		Settings settings;
		ofImage dotImage; 
	
		MonoSample left;
		MonoSample right;
};
