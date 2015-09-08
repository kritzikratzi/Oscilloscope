#include "ofApp.h"
#include "util/split.h"
#include <Poco/Mutex.h>
#include <Poco/TemporaryFile.h>
#include "globals.h"
#include <cctype> 
Poco::Mutex mutex;
Poco::Mutex updateMutex;

bool applicationRunning = false;
ofImage table;


//--------------------------------------------------------------
void ofApp::setup(){
	table.load("testomat.png");
	changed = false;
	clearFbos = false;
	lastMouseMoved = 0;
	ofSetVerticalSync(true);
	ofBackground(0);
	ofSetBackgroundAuto(false);
	dotImage.setUseTexture(true);
	dotImage.allocate(64, 64, OF_IMAGE_COLOR_ALPHA);
	dotImage.getTexture().setTextureWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	dotImage.load( "dot.png" );
	shader.setGeometryOutputCount(4);
	shaderLoader.setup(&shader, "shaders/osci" );
	blur.load("shaders/blur");
	
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
	
	changed = false;
	shapeMesh.clear();
	shapeMesh.setMode(OF_PRIMITIVE_POINTS);
	shapeMesh.enableColors();
	
	const int bufferSize = 512*4;
	static float * leftBuffer = new float[bufferSize];
	static float * rightBuffer = new float[bufferSize];
	static ofPoint a, b, ab, p;
	static ofColor lastColor;
	float S = MIN(ofGetWidth()/2,ofGetHeight()/2)*globals.scale;

	if( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
		changed = true;
		while( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
			memset(leftBuffer,0,bufferSize*sizeof(float));
			memset(rightBuffer,0,bufferSize*sizeof(float));
			
			left.addTo(leftBuffer, 1, bufferSize);
			right.addTo(rightBuffer, 1, bufferSize);
			left.peel(bufferSize);
			right.peel(bufferSize);
			
			ofColor col = ofColor::fromHsb(globals.hue*255/360, 255, 255);
			
			float dMax = 0;
			if( shapeMesh.getVertices().size() < bufferSize*2 ){
				for( int i = 0; i < bufferSize; i++ ){
					b = ofPoint( leftBuffer[i], rightBuffer[i] );
					ab = b-a;
					// here, have a bunch of magical numbers!
					float dist = ab.length();
					const float alph = powf(0.08,0.18);
					float E = powf(ofClamp( 1-40*dist, 0, 1),40);
					float alpha = 255*MAX(E,alph);
					float erosion = E*20;
					
					//lastColor = ofColor( 255, erosion, erosion, 5 );
//					lastColor = ofColor( erosion, 204, 255, 4 );
//					lastColor = ofColor( erosion, 255, erosion, 4 );
					lastColor = ofColor::fromHsb(globals.hue, 255-erosion, 255, globals.intensity*8);
					
					dMax = MAX(dist, dMax);
					
					float N = globals.numPts;
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
	ofClear(0,255);
	
	if( !fbo.isAllocated() || fbo.getWidth() != ofGetWidth() || fbo.getHeight() != ofGetHeight() ){
		fbo.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA,4);
		fbb.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA,4);
		fbo.begin();
		ofClear(0,255);
		fbo.end();
		fbb.begin();
		ofClear(0,255);
		fbb.end(); 
	}
	
	if( changed && globals.player.isPlaying ){
		fbb.begin();
		ofEnableAlphaBlending();
		ofSetColor(255);
		blur.begin();
		blur.setUniform1f("blurDist", 29);
		blur.setUniform1f("blurAmnt", globals.blur/255.0);
		fbo.draw(0,0);
		blur.end();
		fbb.end();
		
		fbo.begin();
		ofEnableAlphaBlending();
		ofSetColor(255);
		blur.begin();
		blur.setUniform1f("blurDist", 5);
		blur.setUniform1f("blurAmnt", globals.blur/255.0);
		fbb.draw(0,0);
		blur.end();
		fbo.end();
		
		
		fbo.begin();
		//	glEnable(GL_BLEND);
		//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		ofSetColor( 0, (1-globals.afterglow)*50 );
		ofFill();
		ofDrawRectangle( 0, 0, ofGetWidth(), ofGetHeight() );
		
//		fbb.draw(0,0);
		//	ofClear(0,0,0,0);
		//	glClearColor(0, 0, 0, 0);
		//	glClear(GL_COLOR_BUFFER_BIT);
		ofEnableAlphaBlending();
		
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
		
		shapeMesh.enableColors();
		glPointSize(2*mui::MuiConfig::scaleFactor);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		shader.begin();
		shader.setUniform1f("width", globals.strokeWeight );
		shader.setUniform1f("height", globals.strokeWeight );
		shader.setUniformTexture("tex", dotImage.getTextureReference(), 1);
		ofSetColor(255);
		shapeMesh.draw();
		shader.end();
		glDisable(GL_BLEND);
		ofEnableAlphaBlending();
		
		
		ofPopMatrix();
		fbo.end();
	}
	
	fbo.draw(0,0);
}

void exit_from_c(){
	exit(0); 
}

void ofApp::exit(){
	stopApplication();
	exit_from_c();
}


//----------------------------------------------------------	----
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
	
	if( key == 'r' ){
		clearFbos = true;
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
	osciView->layout();
	osciView->x = w/mui::MuiConfig::scaleFactor/2 - osciView->width/2;
	osciView->y = h/mui::MuiConfig::scaleFactor - osciView->height - 20;
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
		
		AudioAlgo::scale(output, globals.outputVolume, nChannels*bufferSize);
		
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
