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
	
	soundStream.setDeviceID( globals.deviceId );
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
	shapeMesh.setMode(OF_PRIMITIVE_POINTS);
	shapeMesh.enableColors();
	
	const int bufferSize = 512;
	static float * leftBuffer = new float[bufferSize];
	static float * rightBuffer = new float[bufferSize];
	static ofPoint a, b, ab, p;
	static ofColor lastColor;
	float S = MIN(ofGetWidth()/2,ofGetHeight()/2)*globals.scale;

	if( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
		while( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
			memset(leftBuffer,0,bufferSize*sizeof(float));
			memset(rightBuffer,0,bufferSize*sizeof(float));
			
			left.addTo(leftBuffer, 1, bufferSize);
			right.addTo(rightBuffer, 1, bufferSize);
			left.peel(bufferSize);
			right.peel(bufferSize);
			
			float dMax = 0;
			if( shapeMesh.getVertices().size() < bufferSize*8 ){
				for( int i = 0; i < bufferSize; i++ ){
					b = ofPoint( leftBuffer[i], rightBuffer[i] );
					ab = b-a;
					// here, have a bunch of magical numbers!
					float dist = ab.length();
					const float alph = powf(0.08,0.18);
					float E = powf(ofClamp( 1-40*dist, 0, 1),40);
					float alpha = 255*MAX(E,alph);
					float erosion = 150*E;
					
					//lastColor = ofColor( 255, erosion, erosion, 5 );
					lastColor = ofColor( erosion, 204, 255, 5 );
					
					dMax = MAX(dist, dMax);

					float N = 100;
					for( int j = 0; j < N; j++ ){
						float alpha = j/(float)N;
						p = a + ab*alpha;
						shapeMesh.addVertex(p*S);
						shapeMesh.addColor(lastColor);
					}
					
					a = b;
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableAlphaBlending();
	ofSetColor( 0, 40 );
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
/*
	ofSetColor(50, 255, 50, 30);
//	shapeMesh.disableColors();
	glPointSize(20.0);
	shapeMesh.draw();
	
	ofSetColor(50, 255, 50, 50);
//	shapeMesh.disableColors();
	glPointSize(5.0);
	shapeMesh.draw();
	
	ofSetColor(75, 255, 75, 50);
//	shapeMesh.disableColors();
	glPointSize(2.5);
	shapeMesh.draw();
	*/
	shapeMesh.enableColors();
	glPointSize(5*mui::MuiConfig::scaleFactor);
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
