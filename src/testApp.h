#pragma once

#include "ofMain.h"
#include "sounddevices.h"
#include "MUI.h" 
#include "ConfigView.h"
#include "MeshView.h"
#include "Audio.h"


class testApp : public ofBaseApp{

	public:

		void setup();
		void startApplication();
		void stopApplication();
		void update();
		void update( ofMesh &shapeMesh, MonoSample &left, MonoSample &right, float &lx, float &ly );
		void draw();
		void draw( ofMesh &shapeMesh, int index );
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
		MeshView * meshView;
		ofMesh shapeMesh1;
		ofMesh shapeMesh2;
		Settings settings;
		ofImage dotImage; 

		float lx1, ly1, lx2, ly2; 
	
		MonoSample left1;
		MonoSample right1;
		MonoSample left2;
		MonoSample right2;
};
