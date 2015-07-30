#include "ofApp.h"
#include "util/split.h"
#include <Poco/Mutex.h>
#include <Poco/TemporaryFile.h>
#include "globals.h"
#include <cctype> 
Poco::Mutex mutex;
Poco::Mutex updateMutex;

bool applicationRunning = false;


//--------------------------------------------------------------
void ofApp::setup(){
	lastMouseMoved = 0;
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
	
	globals.loadFromFile();
	globals.player.loadSound( "konichiwa.wav" );
	globals.player.setLoop(true);
	globals.player.stop(); 
	
	configView = new ConfigView();
	configView->fromGlobals();
	if( globals.autoDetect ){
		configView->autoDetect();
	}

	root->add( configView );
	
	osciView = new OsciView();
	osciView->visible = false;
	root->add( osciView );
	
	left.loop = false;
	right.loop = false;

	
	if( globals.autoDetect ){
		startApplication();
	}

	
	windowResized(ofGetWidth(), ofGetHeight());
}


void ofApp::startApplication(){
	if( applicationRunning ) return;
	applicationRunning = true;
	
	left.play();
	right.play();

	configView->toGlobals();
	globals.saveToFile();
	configView->visible = false;
	osciView->visible = true;
	
	if( globals.autoDetect ){
		cout << "Running auto-detect for sound cards" << endl;
		getDefaultRtOutputParams( globals.deviceId, globals.sampleRate, globals.bufferSize, globals.numBuffers );
	}
	
	//if you want to set the device id to be different than the default
	cout << "Opening Sound Card: " << endl;
	cout << "    Sample rate: " << globals.sampleRate << endl;
	cout << "    Buffer size: " << globals.bufferSize << endl;
	cout << "    Num Buffers: " << globals.numBuffers << endl;
	
//	soundStream.setDeviceID( globals.deviceId );
	soundStream.setup(this, 2, 0, globals.sampleRate, globals.bufferSize, globals.numBuffers);
	globals.player.setupAudioOut(2, globals.sampleRate);
}


void ofApp::stopApplication(){
	configView->fromGlobals();
	globals.flipXY = globals.flipXY;
	globals.invertX = globals.invertX;
	globals.invertY = globals.invertY;

	globals.scale = osciView->scaleSlider->value;
	globals.saveToFile();
	
	if( !applicationRunning ) return;
	applicationRunning = false;
	soundStream.stop();
	soundStream = ofSoundStream();
	configView->visible = true;
	osciView->visible = false;
}



//--------------------------------------------------------------
void ofApp::update(){
	
	// nasty hack. OF seems to get width+height wrong on the first frame.
	if( ofGetFrameNum() == 1 ){
		windowResized(ofGetWidth(), ofGetHeight());
	}
	
/*	if( ofGetElapsedTimeMillis()-lastMouseMoved > 5000 && globals.player.isPlaying ){
		osciView->visible = false;
	}*/

	if( !applicationRunning ){
		ofShowCursor();
		return;
	}
	if( osciView->visible ) ofShowCursor();
	else ofHideCursor();
		
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
		
		float S= ofGetWidth()/2*osciView->scaleSlider->value;
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
void ofApp::draw(){
	ofEnableAlphaBlending();
	ofSetColor( 0, 80 );
	ofFill();
	ofRect( 0, 0, ofGetWidth(), ofGetHeight() );
	
	ofPushMatrix();
	ofTranslate(ofGetWidth()/2, ofGetHeight()/2 );
	
	int scaleX = 1;
	int scaleY = -1;
	if( globals.invertX ) scaleX *= -1;
	if( globals.invertY ) scaleY *= -1;
	
	
	ofScale( scaleX, scaleY );
	if( globals.flipXY ){
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
}

void exit_from_c(){
	exit(0); 
}

void ofApp::exit(){
	stopApplication();
	exit_from_c();
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	lastMouseMoved = ofGetElapsedTimeMillis();
	key = std::tolower(key);
	
	if( key == '\t' && !configView->isVisibleOnScreen()){
		osciView->visible = !osciView->visible;
	}

	if( key == 'f' || key == OF_KEY_RETURN || key == OF_KEY_F11 ){
		// nasty!
		osciView->fullscreenButton->clickAndNotify(); 
	}
	
	if( key == ' '  ){
		osciView->playButton->clickAndNotify();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	lastMouseMoved = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	lastMouseMoved = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	lastMouseMoved = ofGetElapsedTimeMillis();
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	osciView->width = min(500,w/mui::MuiConfig::scaleFactor);
	osciView->height = 160;
	osciView->x = w/mui::MuiConfig::scaleFactor/2 - osciView->width/2;
	osciView->y = h/mui::MuiConfig::scaleFactor - osciView->height - 20;
	osciView->layout();
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
	if( !globals.player.isLoaded ){
		left.append(input, bufferSize,2);
		right.append(input+1,bufferSize,2);
	}
}

void ofApp::audioOut( float * output, int bufferSize, int nChannels ){
	memset(output, 0, bufferSize*nChannels);
	if( globals.player.isLoaded ){
		globals.player.audioOut(output, bufferSize, nChannels);
		left.append(output, bufferSize,2);
		right.append(output+1,bufferSize,2);
	}
	else{
		
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
	if( msg.message == "start-pressed" ){
		startApplication();
	}
	else if( msg.message == "stop-pressed" ){
		stopApplication();
	}
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	cout << "files: " << endl;
	for( vector<string>::iterator it = dragInfo.files.begin();it != dragInfo.files.end(); ++it ){
		cout << *it << endl;
	}
	
	if( dragInfo.files.size() >= 1 ){
		globals.player.loadSound(dragInfo.files[0]);
	}
	
}
