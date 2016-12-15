#include "ofApp.h"
#include "util/split.h"
#include <Poco/Mutex.h>
#include <Poco/TemporaryFile.h>
#include "globals.h"
#include <cctype> 
Poco::Mutex mutex;
Poco::Mutex updateMutex;

bool applicationRunning = false;

#define EPS 1E-6

//--------------------------------------------------------------
void ofApp::setup(){
	mui::MuiConfig::fontSize = 16;
	showInfo = false;
	dropped = 0;
	changed = false;
	clearFbos = false;
	lastMouseMoved = 0;
	exporting = 0;
	
	applicationRunning = false; 
	ofSetVerticalSync(true);
	ofBackground(0);
	ofSetBackgroundAuto(false);
	
	// shaderLoader handles this for us
//	shader.setGeometryInputType(GL_LINES);
//	shader.setGeometryOutputType(GL_QUADS);
//	shader.setGeometryOutputCount(4);
	shaderLoader.setup(&shader, "shaders/osci");
	
	ofSetFrameRate(60);
	
	root = new mui::Root();
	
	globals.player.loadSound( ofxToReadonlyDataPath("konichiwa.wav") );
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
	cout << "starting ..." << endl; 
	left.play();
	right.play();

	configView->toGlobals();
	globals.saveToFile();
	configView->visible = false;
	osciView->fromGlobals();
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
	globals.player.setupAudioOut(2, globals.sampleRate, true);
}


void ofApp::stopApplication(){
	configView->toGlobals();
	globals.saveToFile();
	
	if( !applicationRunning ) return;
	applicationRunning = false;
	soundStream.stop();
	soundStream = ofSoundStream();
	micStream.stop();
	globals.micActive = false;
	configView->visible = true;
	osciView->visible = false;
}



//--------------------------------------------------------------
void ofApp::update(){
	
	if( ofGetMousePressed() ){
		lastMouseMoved = ofGetElapsedTimeMillis(); 
	}

	/////////////////////////////////////////////////
	// take care of hiding / showing the ui
	if( ofGetElapsedTimeMillis()-lastMouseMoved > 1500 && ( globals.player.isPlaying || globals.micActive ) ){
		// this is not the greatest solution, but hey ho, it works ...
		mui::Container * res = root->findChildAt(ofGetMouseX()/mui::MuiConfig::scaleFactor, ofGetMouseY()/mui::MuiConfig::scaleFactor);
		bool foundOsciView = false;
		while( res != NULL ){
			if( res == osciView ){
				foundOsciView = true;
				break;
			}
			res = res->parent;
		}
		if( !foundOsciView ){
			osciView->visible = false;
		}
	}

	if( !applicationRunning ){
		ofShowCursor();
		return;
	}

	if( ofGetElapsedTimeMillis()-lastMouseMoved < 3000 && osciView->visible == false ){
		osciView->visible = true;
	}

	if( osciView->visible ) ofShowCursor();
	else ofHideCursor();
	
	/////////////////////////////////////////////////
	
	// are we not export?
	if( exporting == 0 ){
		int rate = max(globals.sampleRate/4,(int)std::round(globals.sampleRate*globals.timeStretch));
		globals.player.setupAudioOut(2, rate, globals.timeStretch == 1);
	}

	// are we exporting?
	if( exporting == 1 ){
		// make sure the audio callback doesn't interfere with us!
		ofSleepMillis(1000);
		
		// reset drop count. this has no purpose, but gives the user a good feeling
		dropped = 0;
		
		// resize&clear fbo
		fbo.allocate(globals.exportWidth, globals.exportHeight, GL_RGBA);
		fbo.begin();
		ofClear(0,255);
		fbo.end();
		
		// reset player
		exporting = 2;
		globals.player.beginSync(512);
		globals.player.setPositionMS(0);
		globals.player.setLoop(false);
		globals.player.play();
		exportFrameNum = -1;
	}
	
	if( exporting == 2 ){
		// compute target time for this frame, then
		// we just eat the buffer into nirvana.
		// our funky player will automatically place
		exportFrameNum ++;
		int targetTimeMS = exportFrameNum*1000.0/globals.exportFrameRate;
		const int bufferSize = 512;
		static float * output = NULL;
		if( output == NULL ) output = new float[2*bufferSize];
		
		int len;
		do{
			len = globals.player.audioOutSync(output, bufferSize, 2);
		}
		while( globals.player.getPositionMS() < targetTimeMS && len > 0 );
		
		if( len == 0 ){
			// save this frame, then end it!
			exporting = 3;
		}
		
	}
	else if( exporting == 3 ){
		exporting = 0;
		globals.player.endSync();
		globals.player.setLoop(true);
		globals.player.setPositionMS(0);
	}
	

	/////////////////////////////////////////////////
	// copy buffer data to the mesh
	
	changed = false;
	shapeMesh.clear();
	shapeMesh.setMode(OF_PRIMITIVE_TRIANGLES);
	shapeMesh.enableColors();
	
	int bufferSize = (exporting==0?2084:256);
	static float * leftBuffer = new float[bufferSize];
	static float * rightBuffer = new float[bufferSize];

	// party mode
	//globals.hue += ofGetMouseX()*100/ofGetWidth();
	//globals.hue = fmodf(globals.hue,360);
	
	MonoSample &left = globals.micActive?(this->left):globals.player.left192;
	MonoSample &right = globals.micActive?(this->right):globals.player.right192;
	left.play();
	right.play();
	bool isMono = !globals.micActive && globals.player.isMonoFile;
	
	if( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
		changed = true;
		/*shapeMesh.addVertex(lastA0Vert);
		shapeMesh.addColor(lastA0Col);
		shapeMesh.addVertex(lastA1Vert);
		shapeMesh.addColor(lastA1Col);*/
		
		float uSize = globals.strokeWeight / 1000.0;
		auto addPt = [&]( ofVec2f p0, ofVec2f p1 ){
			ofVec2f dir = p1 - p0;
			float z = dir.length();
			if (z > EPS) dir /= z;
			else dir = ofVec2f(1.0, 0.0);
			
			dir *= uSize;
			ofVec2f norm(-dir.y, dir.x);
			ofVec2f xy(-uSize, -uSize);
			
			shapeMesh.addVertex(ofVec3f(p0-dir-norm));
			shapeMesh.addColor(ofFloatColor(-uSize, -uSize, z));
			
			shapeMesh.addVertex(ofVec3f(p0-dir+norm));
			shapeMesh.addColor(ofFloatColor(-uSize, uSize, z));
			
			shapeMesh.addVertex(ofVec3f(p1+dir-norm));
			shapeMesh.addColor(ofFloatColor(z+uSize, -uSize, z));
			
			
			
			shapeMesh.addVertex(ofVec3f(p0-dir+norm));
			shapeMesh.addColor(ofFloatColor(-uSize, uSize, z));
			
			shapeMesh.addVertex(ofVec3f(p1+dir-norm));
			shapeMesh.addColor(ofFloatColor(z+uSize, -uSize, z));
			
			shapeMesh.addVertex(ofVec3f(p1+dir+norm));
			shapeMesh.addColor(ofFloatColor(z+uSize, +uSize, z));
		};
		
		while( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
			if(isMono){
				memset(rightBuffer,0,bufferSize*sizeof(float));
				right.addTo(rightBuffer, 1, bufferSize);
				for( int i = 0; i < bufferSize; i++ ){
					leftBuffer[i] = -1+2*i/(float)bufferSize;
				}
			}
			else{
				memset(leftBuffer,0,bufferSize*sizeof(float));
				memset(rightBuffer,0,bufferSize*sizeof(float));
				left.addTo(leftBuffer, 1, bufferSize);
				right.addTo(rightBuffer, 1, bufferSize);
			}
			
			if( shapeMesh.getVertices().size() < bufferSize*16 || exporting ){
				
				addPt(last,{leftBuffer[0],rightBuffer[0]});
				last = {leftBuffer[bufferSize-1],rightBuffer[bufferSize-1]};
				
				for( int i = 1; i < bufferSize; i++ ){
					ofVec2f p0(leftBuffer[i-1], rightBuffer[i-1]);
					ofVec2f p1(leftBuffer[i  ], rightBuffer[i  ]);
					addPt(p0,p1);
				}
				
			}
			else{
				dropped ++;
			}
			
			
			left.peel(bufferSize);
			right.peel(bufferSize);
		}
	}
}

ofMatrix4x4 ofApp::getViewMatrix() {
	ofMatrix4x4 viewMatrix = ofMatrix4x4(
		globals.scale, 0.0, 0.0, 0.0, 
		0.0, -globals.scale, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0);

	if (globals.invertX) viewMatrix(0,0) *= -1;
	if (globals.invertY) viewMatrix(1,1) *= -1;

	if (globals.flipXY) {
		viewMatrix = ofMatrix4x4(
			0.0, 1.0, 0.0, 0.0, 
			1.0, 0.0, 0.0, 0.0, 
			0.0, 0.0, 1.0, 0.0, 
			0.0, 0.0, 0.0, 1.0 ) * viewMatrix;
	}

	ofMatrix4x4 aspectMatrix; // identity matrix
	float aspectRatio = float(fbo.getWidth()) / float(fbo.getHeight());
	if (aspectRatio > 1.0) {
		aspectMatrix(0,0) /= aspectRatio;
	}
	else {
		aspectMatrix(1,1) *= aspectRatio;
	}

	return viewMatrix * aspectMatrix;
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0,255);
	
	if( !fbo.isAllocated() || fbo.getWidth() != ofGetWidth() || fbo.getHeight() != ofGetHeight() ){
		int w = ofGetWidth(); 
		int h = ofGetHeight();
		if( exporting ){
			// no need to do anything, fbo is managed by someone else
		}
		else if( w == 0 || h == 0 ){
			//what is happening???
			while( globals.player.left192.totalLength > 4096 && globals.player.right192.totalLength > 4096 ){
				globals.player.left192.peel(4096);
				globals.player.right192.peel(4096);
			}
		}
		else{
			cout << "allocating framebuffer with " << w << ", " << h << endl; 
			fbo.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA);
			fbo.begin();
			ofClear(0,255);
			fbo.end();
		}
	}
	
	if( changed && ( globals.player.isPlaying || globals.micActive ) ){
		fbo.begin();
		ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
		ofSetColor( 0, (1-globals.afterglow)*255 );
		ofFill();
		ofDrawRectangle( 0, 0, fbo.getWidth(), fbo.getHeight() );
	
		ofEnableAlphaBlending();
		ofMatrix4x4 viewMatrix = getViewMatrix();

//      TODO: draw the cross section
//		ofSetColor(255, 0, 0, 25);
//		ofDrawLine( -10, 0, 10, 0 );
//		ofDrawLine( 0, -10, 0, 10 );
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		shader.begin();
		shader.setUniform1f("uSize", globals.strokeWeight / 1000.0);
		shader.setUniform1f("uIntensity", globals.intensity/sqrtf(globals.timeStretch));
		shader.setUniformMatrix4f("uMatrix", viewMatrix);
		shader.setUniform1f("uHue", globals.hue );
		ofSetColor(255);
		shapeMesh.draw();
		shader.end();
		ofEnableAlphaBlending();

		fbo.end();
	}
	
	ofSetColor(255);
	fbo.draw(0,0);
	
	if( exporting >= 2 ){
		string filename = ofToDataPath(exportDir + "/" + ofToString(exportFrameNum, 5, '0') + ".png");
		ofPixels pixels;
		fbo.readToPixels(pixels);
		ofSaveImage(pixels, filename);
	}
	
	if( showInfo || exporting > 0 ){
		ofSetColor(exporting>0?255:100);
		ofDrawBitmapString("Dropped: " + ofToString(dropped), 10, 20 );
		ofDrawBitmapString("FPS:     " + ofToString(ofGetFrameRate(),0), 10, 40 );
		
		if( exporting > 0 ){
			unsigned long long totalFrames = 1+globals.player.duration*globals.exportFrameRate/1000;
			int pct = exportFrameNum*100/totalFrames;
			ofDrawBitmapString("Format:  " + ofToString(globals.exportWidth) + " x " + ofToString(globals.exportHeight) + " @ " + ofToString(globals.exportFrameRate) + "fps (change in " + ofxToReadWriteableDataPath(")settings.txt") + ")", 10, 60);
			ofDrawBitmapString("Export:  " + ofToString(pct) + "%  (" + ofToString(exportFrameNum) + "/" + ofToString(totalFrames) + ")", 10, 80 );
			if( (exportFrameNum%10) < 5 ){
				ofSetColor(255,0,0);
			}
			else{
				ofSetColor(255);
			}
			ofFill();
			ofDrawEllipse(20, 100, 20, 20);
		}
	}
}

void ofApp::exit(){
	stopApplication();
	std::exit(0);
}


//----------------------------------------------------------	----
void ofApp::keyPressed  (int key){
	key = std::tolower(key);
	
	if( key == '\t' && !configView->isVisibleOnScreen()){
		osciView->visible = !osciView->visible;
		if( osciView->visible ) lastMouseMoved = ofGetElapsedTimeMillis();
		else lastMouseMoved = 0;
	}

	if( key == 'f' || key == OF_KEY_RETURN || key == OF_KEY_F11 ){
		// nasty!
		osciView->fullscreenButton->clickAndNotify(); 
	}
	
	if( key == OF_KEY_ESC ){
		osciView->fullscreenButton->clickAndNotify(false);
	}
	
	if( key == ' '  ){
		osciView->playButton->clickAndNotify();
	}
	
	if( key == 'r' ){
		clearFbos = true;
	}
	
	if( key == 'i' ){
		showInfo ^= true;
	}
	
	if( key == 'e' && exporting == 0 ){
		ofFileDialogResult res = ofSystemSaveDialog("images", "Create destination folder" );
		if( res.bSuccess ){
			exportDir = res.filePath;
			ofDirectory dir(exportDir);
			dir.create();
			if( dir.exists() && !dir.isDirectory() ){
				// don't export!
			}
			else{
				exporting = 1;
			}
		}
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
	cout << "resize to " << w << "," << h << endl; 
	osciView->width = min(500,w/mui::MuiConfig::scaleFactor);
	osciView->layout();
	osciView->x = w/mui::MuiConfig::scaleFactor/2 - osciView->width/2;
	osciView->y = h/mui::MuiConfig::scaleFactor - osciView->height - 60;
	cout << "visible?" << osciView << "::" << osciView->visible << endl; 
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
	if( globals.micActive ){
		left.append(input, bufferSize,2);
		right.append(input+1,bufferSize,2);
	}
}

void ofApp::audioOut( float * output, int bufferSize, int nChannels ){
	if( fileToLoad != "" ){
		globals.timeStretch = 1.0;
		globals.player.loadSound(fileToLoad);
		osciView->timeStretchSlider->slider->value = 1.0;
		setWindowRepresentedFilename(fileToLoad);
		fileToLoad = "";
	}
	
	memset(output, 0, bufferSize*nChannels);
	if( globals.player.isLoaded && exporting == 0 && !globals.micActive ){
		globals.player.audioOut(output, bufferSize, nChannels);
		AudioAlgo::scale(output, globals.outputVolume, nChannels*bufferSize);
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
	else if( msg.message == "start-mic" ){
		if( exporting != 0 ) return;
		globals.player.stop();
		
		if( globals.micActive ){
			micStream.stop();
			micStream = ofSoundStream();
		}
		
		micStream.setDeviceID(globals.micDeviceId);
		micStream.setup(this, 0, 2, globals.sampleRate, globals.bufferSize, globals.numBuffers);
		micStream.start(); 
		globals.micActive = true;
	}
	else if( msg.message == "stop-mic" ){
		micStream.stop();
		micStream = ofSoundStream(); 
		globals.micActive = false;
	}
	else if( msg.message.substr(0,5) == "load:" ){
		fileToLoad = msg.message.substr(5);
	}
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	for( vector<string>::iterator it = dragInfo.files.begin();it != dragInfo.files.end(); ++it ){
		cout << *it << endl;
	}
	
	if( dragInfo.files.size() >= 1 ){
		// this runs on a separate thread.
		// we have to be careful not to make a mess!
		fileToLoad = dragInfo.files[0];
	}
	
}
