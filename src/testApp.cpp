#include "testApp.h"
#include "util/split.h"
#include <Poco/Mutex.h>
#include <Poco/TemporaryFile.h>
#include "ofxIniSettings.h"

Poco::Mutex mutex;
Poco::Mutex updateMutex;

bool applicationRunning = false;


//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	ofBackground(0);
	ofSetBackgroundAuto(false);
	dotImage.allocate(64, 64, OF_IMAGE_COLOR_ALPHA);
	dotImage.loadImage( "dot.png" );
	
	
	cout << "Available Sound Devices: " << endl;
	soundStream.listDevices();
	cout << "=============================" << endl << endl;
	
	
	vector<RtAudio::DeviceInfo> devices = listRtSoundDevices();
	ofSetFrameRate(60);
	
	root = new mui::Root();
	
	settings.loadFromFile();
	
	configView = new ConfigView();
	configView->fromSettings( settings );
	root->add( configView );
	
	osciView = new OsciView();
	osciView->visible = false;
	root->add( osciView );
	
	left.loop = false;
	right.loop = false;

	
	if( settings.autoDetect ){
		startApplication();
	}

}


void testApp::startApplication(){
	if( applicationRunning ) return;
	applicationRunning = true;
	
	left.play();
	right.play();

	configView->toSettings( settings );
	osciView->flipXY->selected = settings.flipXY;
	osciView->invertX->selected = settings.invertX;
	osciView->invertY->selected = settings.invertY;
	osciView->scaleSlider->slider->value = settings.scale;

	settings.saveToFile();
	configView->visible = false;
	osciView->visible = true;
	
	if( settings.autoDetect ){
		cout << "Running auto-detect for sound cards" << endl;
		getDefaultRtOutputParams( settings.deviceId, settings.sampleRate, settings.bufferSize, settings.numBuffers );
	}
	
	//if you want to set the device id to be different than the default
	cout << "Opening Sound Card: " << endl;
	cout << "    Sample rate: " << settings.sampleRate << endl;
	cout << "    Buffer size: " << settings.bufferSize << endl;
	cout << "    Num Buffers: " << settings.numBuffers << endl;
	
	ofSoundPlayer player; 
//	soundStream.setDeviceID( settings.deviceId );
	soundStream.setup(this, 2, 0, settings.sampleRate, settings.bufferSize, settings.numBuffers);
	filePlayer.setupAudioOut(2, settings.sampleRate);
}


void testApp::stopApplication(){
	configView->fromSettings(settings);
	settings.flipXY = osciView->flipXY->selected;
	settings.invertX = osciView->invertX->selected;
	settings.invertY = osciView->invertY->selected;
	settings.scale = osciView->scaleSlider->slider->value;
	settings.saveToFile();
	
	if( !applicationRunning ) return;
	applicationRunning = false;
	soundStream.stop();
	soundStream = ofSoundStream();
	configView->visible = true;
	osciView->visible = false;
}



//--------------------------------------------------------------
void testApp::update(){
	root->handleUpdate();
	if( !applicationRunning ) return;
	
	shapeMesh.clear();
	shapeMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
	shapeMesh.enableColors();
	
	static float * leftBuffer = new float[512];
	static float * rightBuffer = new float[512];
	
	while( left.totalLength >= 512 && right.totalLength >= 512 ){
		memset(leftBuffer,0,512*sizeof(float));
		memset(rightBuffer,0,512*sizeof(float));
		
		left.addTo(leftBuffer, 1, 512);
		right.addTo(rightBuffer, 1, 512);
		left.peel(512);
		right.peel(512);
		
		float S= ofGetWidth()/2*osciView->scaleSlider->slider->value;
		float x2 = leftBuffer[1];
		float y2 = rightBuffer[1];
		float x1, y1;
		if( shapeMesh.getVertices().size() < 2048*4 ){
			for( int i = 0; i < 512; i++ ){
				x1 = leftBuffer[i];
				y1 = rightBuffer[i];
				
				float d = ofDist(x1, y1, x2, y2);
				shapeMesh.addVertex(ofVec3f(x1*S, y1*S,0));
				shapeMesh.addColor(ofColor(200, 255, 200, 255*(1-powf(d,0.077))));
	//			shapeMesh.addColor(ofColor(i/2, 255, 255, 255));
				x2 = x1;
				y2 = y1;
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	ofEnableAlphaBlending();
	ofSetColor( 0, 80 );
	ofFill();
	ofRect( 0, 0, ofGetWidth(), ofGetHeight() );
	
	ofShowCursor();
	ofPushMatrix();
	ofTranslate(ofGetWidth()/2, ofGetHeight()/2 );
	
	int scaleX = 1;
	int scaleY = -1;
	if( osciView->invertX->selected ) scaleX *= -1;
	if( osciView->invertY->selected ) scaleY *= -1;
	
	
	ofScale( scaleX, scaleY );
	if( osciView->flipXY->selected ){
		ofRotate(-90);
		ofScale( -1, 1 );
	}
	
	ofSetColor(255, 0, 0, 25);
	ofLine( -10, 0, 10, 0 );
	ofLine( 0, -10, 0, 10 );

	ofSetColor(50, 255, 50, 30);
	shapeMesh.disableColors();
	ofSetLineWidth(20.0);
	shapeMesh.draw();
	
	ofSetColor(50, 255, 50, 50);
	shapeMesh.disableColors();
	ofSetLineWidth(5.0);
	shapeMesh.draw();
	
	ofSetColor(75, 255, 75, 50);
	shapeMesh.disableColors();
	ofSetLineWidth(2.5);
	shapeMesh.draw();
	
	shapeMesh.enableColors();
	ofSetLineWidth(1.0);
	shapeMesh.draw();
	
/*	vector<ofVec3f> verts = shapeMesh.getVertices();
	vector<ofVec3f>::iterator it = verts.begin();
	ofSetColor(255,20);
	int i = 0;
	while( it != verts.end() ){
		if( i % 10 == 0 )
			dotImage.draw((*it).x-dotImage.width/2, (*it).y-dotImage.height/2);
		++i;
		++it;
	}*/
//	ofDisableBlendMode();
	
	ofPopMatrix();
	
	root->handleDraw();
}

void testApp::exit(){
	stopApplication();
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	if( root->handleKeyPressed( key ) ) return;
	
	if( key == '\t' && !configView->isVisibleOnScreen()){
		osciView->visible = !osciView->visible;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
	if( root->handleKeyReleased( key ) ) return;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	if( root->handleMouseMoved( x, y ) ) return;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	if( root->handleMouseDragged( x, y, button ) ) return;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	if( root->handleMousePressed( x, y, button ) ) return;
}


//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	if( root->handleMouseReleased( x, y, button ) ) return;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){
	if( !filePlayer.isLoaded ){
		left.append(input, bufferSize,2);
		right.append(input+1,bufferSize,2);
	}
}

void testApp::audioOut( float * output, int bufferSize, int nChannels ){
	memset(output, 0, bufferSize*nChannels);
	if( filePlayer.isLoaded ){
		filePlayer.audioOut(output, bufferSize, nChannels);
		left.append(output, bufferSize,2);
		right.append(output+1,bufferSize,2);
	}
	else{
		
	}
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
	if( msg.message == "start-pressed" ){
		startApplication();
	}
	else if( msg.message == "stop-pressed" ){
		stopApplication();
	}
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
	cout << "files: " << endl;
	for( vector<string>::iterator it = dragInfo.files.begin();it != dragInfo.files.end(); ++it ){
		cout << *it << endl;
	}
	
	if( dragInfo.files.size() >= 1 ){
		filePlayer.loadSound(dragInfo.files[0]); 
	}
	
}
