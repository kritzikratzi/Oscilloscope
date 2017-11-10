#include "ofApp.h"
#include "util/split.h"
#include <Poco/Mutex.h>
#include <Poco/TemporaryFile.h>
#include "globals.h"
#include <cctype> 
#include "ofxNative.h"

Poco::Mutex mutex;
Poco::Mutex updateMutex;

bool applicationRunning = false;

//--------------------------------------------------------------
void ofApp::setup(){
	OsciMesh::init();
	
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
	
	ofSetFrameRate(60);
	
	root = new mui::Root();
	
	globals.player.loadSound(ofxToReadonlyDataPath("konichiwa.wav"));
//	globals.player.loadSound(ofxToReadonlyDataPath("c:\\Users\\hansi\\Desktop\\3dbounce.wav"));
	globals.player.setLoop(false);
	globals.player.stop();
	globals.player.onEnd = [&](){
		// see if we have a "next" item
		//git-forbid do this on the right thread!
		if(playlistEnable){
			auto next = playlist->getNextItem(globals.currentlyPlayingItem);
			if(next.first > 0){
				fileToLoad = next.second;
				globals.currentlyPlayingItem = next.first;
			}
			else if(globals.currentlyPlayingItem == 0){
				globals.player.setPositionMS(0);
			}
			else{
				globals.currentlyPlayingItem = 0;
			}
		}
		else{
			globals.player.setPositionMS(0);
			globals.player.play();
		}
	};
	
	configView = new ConfigView();
	configView->fromGlobals();
	if( globals.autoDetect ){
		configView->autoDetect();
	}

	root->add( configView );
	
	osciView = new PlayerOverlay();
	osciView->visible = false;
	root->add( osciView );
	
	playlist = new Playlist();
	root->add(playlist);
	{
		ifstream in(ofxToReadWriteableDataPath("playlist.txt"));
		playlist->load(in);
		in.close();
	}
	
	
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
	left.playFrom(0);
	right.playFrom(0);

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
	
	lastMouseMoved = ofGetElapsedTimeMillis();
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
	lastUpdateTime = ofGetElapsedTimeMillis();
	if( nextWindowTitle != "" ){
		setWindowRepresentedFilename(nextWindowTitle);
		nextWindowTitle = ""; 
	}
	
	playlist->visible = osciView->visible && playlistEnable;
	
	bool anythingGoingOn = globals.player.isPlaying || exporting>0;
	if( ofGetMousePressed() || !anythingGoingOn){
		lastMouseMoved = lastUpdateTime;
	}
	
	// reload settings when we went from not focused->focused state
	if(ofGetFrameNum()%30 == 0){
		ofAppGLFWWindow * win = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr());
		if(win){
			bool focused = glfwGetWindowAttrib(win->getGLFWWindow(), GLFW_FOCUSED);
			if(focused && !hadWindowFocus){
				globals.loadFromFile();
				osciView->fromGlobals();
			}
			else if(!focused && hadWindowFocus){
				globals.saveToFile(); 
			}
			hadWindowFocus = focused;
		}
	}

	/////////////////////////////////////////////////
	// take care of hiding / showing the ui
	if( lastUpdateTime-lastMouseMoved > globals.secondsBeforeHidingMenu*1000 ){
		// this is not the greatest solution, but hey ho, it works ...
		bool hovering = osciView->isMouseOver() || playlist->isMouseOver();
		if( !hovering ){
			osciView->visible = false;
			mousePosBeforeHiding.x = ofGetMouseX();
			mousePosBeforeHiding.y = ofGetMouseY();
		}
	}

	if( !applicationRunning ){
		ofShowCursor();
		return;
	}

	ofVec2f mousePos(ofGetMouseX(), ofGetMouseY());
	bool insideWindow = ofRectangle(0,0,ofGetWidth(),ofGetHeight()).inside(mousePos);
	if( lastUpdateTime-lastMouseMoved < globals.secondsBeforeHidingMenu*1000 && osciView->visible == false ){
		bool movedEnough = mousePos.distance(mousePosBeforeHiding) > 10*mui::MuiConfig::scaleFactor;
		if( insideWindow && movedEnough ){
			// okay, we moved enough!
			osciView->visible = true;
		}
	}

	if( osciView->visible || !insideWindow) ofShowCursor();
	else ofHideCursor();
	
	// center the ui
	if (osciView->sideBySide->selected && globals.player.fileType == OsciAvAudioPlayer::QUAD ) {
		osciView->x = ofGetWidth() / mui::MuiConfig::scaleFactor * 1 / 4.0f - osciView->width / 2;
	}
	else {
		osciView->x = ofGetWidth() / mui::MuiConfig::scaleFactor / 2 - osciView->width / 2;
	}
	osciView->y = ofGetHeight() / mui::MuiConfig::scaleFactor - osciView->height - 60;


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
	mesh.clear();
	mesh2.clear();
	
	int bufferSize = (exporting==0?2048:256);
	static float * leftBuffer = new float[2048];
	static float * rightBuffer = new float[2048];
	static float * zmodBuffer = new float[2048];

	// party mode
	//globals.hue += ofGetMouseX()*100/ofGetWidth();
	//globals.hue = fmodf(globals.hue,360);
	
	MonoSample &left = globals.micActive?(this->left):globals.player.left192;
	MonoSample &right = globals.micActive?(this->right):globals.player.right192;
	MonoSample &zMod = globals.player.zMod192; // not available for mic anyways
	
	left.playFrom(0);
	right.playFrom(0);
	zMod.playFrom(0);
	
	bool isMono = !globals.micActive && globals.player.fileType == OsciAvAudioPlayer::MONO;
	bool isZModulated = !globals.micActive && globals.player.fileType == OsciAvAudioPlayer::STEREO_ZMODULATED;
	bool isQuad = !globals.micActive && globals.player.fileType == OsciAvAudioPlayer::QUAD;
	
	osciView->sideBySide->visible = isQuad; 
	osciView->flip3d->visible = isQuad;
	osciView->zModulation->visible = isZModulated;
	
	
	static double T0 = ofGetElapsedTimeMillis();
	double T1 = ofGetElapsedTimeMillis();
	double dt = T1-T0;
	static double avgDt = 0;
	avgDt = MAX(0.95*avgDt,dt);
	double expectedDt = 1000/(float)ofGetTargetFrameRate();
	T0 = T1;
	static uint64_t totalFramesPlayed = 0;
	
	if( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
		changed = true;
		
		float uSize = globals.strokeWeight / 1000.0;
		
		while( left.totalLength >= bufferSize && right.totalLength >= bufferSize ){
			
			int maxVerts = MIN(1,expectedDt/avgDt)*bufferSize*32*ofMap(totalFramesPlayed,0,60,0,1,true);
			totalFramesPlayed ++;

			if( mesh.mesh.getVertices().size() >= maxVerts && !exporting ){
				dropped ++;
				int N = left.totalLength;
				left.peel(N);
				right.peel(N);
				zMod.peel(N);
			}
			else{
				if(isMono){
					right.copyTo(rightBuffer, 1, bufferSize);
					for( int i = 0; i < bufferSize; i++ ){
						leftBuffer[i] = -1+2*i/(float)bufferSize;
					}
				}
				else{
					left.copyTo(leftBuffer, 1, bufferSize);
					right.copyTo(rightBuffer, 1, bufferSize);
				}
			
				int stride = isQuad ? 2 : 1;
				
				float * zBuffer = nullptr;
				if(isZModulated && globals.zModulation){
					zMod.copyTo(zmodBuffer,1,bufferSize);
					zBuffer = zmodBuffer; 
				}
					
				mesh.addLines(leftBuffer, rightBuffer, zBuffer, bufferSize, stride);
				if(isQuad){
					mesh2.addLines(leftBuffer+1, rightBuffer+1, zBuffer, bufferSize, stride);
				}
				
				left.peel(bufferSize);
				right.peel(bufferSize);
				if(isZModulated){
					zMod.peel(bufferSize);
					zMod.setPosition(0); 
				}
			}
		}
	}
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
		else{
			if( w == 0 || h == 0 ){
				//what is happening???
				while( globals.player.left192.totalLength > 4096 && globals.player.right192.totalLength > 4096 ){
					globals.player.left192.peel(4096);
					globals.player.right192.peel(4096);
					globals.player.zMod192.peel(4096); 
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
	}
	
	if( changed && ( globals.player.isPlaying || globals.micActive ) ){
		fbo.begin();
		ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
		ofSetColor( 0, (1-globals.afterglow)*255 );
		ofFill();
		ofDrawRectangle( 0, 0, fbo.getWidth(), fbo.getHeight() );
	
		ofEnableAlphaBlending();
		
		bool sideBySide = osciView->sideBySide->selected;
		bool flip3d = osciView->flip3d->selected;
		ofMatrix4x4 viewMatrix = getViewMatrix(flip3d?1:0,globals.player.fileType == OsciAvAudioPlayer::QUAD);
		ofFloatColor color = ofFloatColor::fromHsb(globals.hue/360.0, 1, 1);

		mesh.uSize = globals.strokeWeight/1000.0;
		mesh.uIntensityBase = max(0.0f,mesh.uIntensity-0.4f)*0.7f-1000.0f*mesh.uSize/500.0f;
		mesh.uIntensity = globals.intensity/sqrtf(globals.timeStretch);
		mesh.uHue = globals.hue;
		mesh.uRgb = globals.player.fileType == OsciAvAudioPlayer::QUAD && !sideBySide?
			(flip3d?ofVec3f(0,1,1):ofVec3f(1,0,0)):
			ofVec3f(color.r,color.g,color.b);
		//mesh.uRgb = ofVec3f(1, 1, 1); 
		mesh.draw(viewMatrix);
		
		if(mesh2.mesh.getNumVertices() > 0){
			mesh2.uSize = mesh.uSize;
			mesh2.uIntensityBase = mesh.uIntensityBase;
			mesh2.uIntensity = mesh.uIntensity;
			mesh2.uHue = globals.hue;
			mesh2.uRgb = sideBySide ? mesh.uRgb : (flip3d ? ofVec3f(1, 0, 0) : ofVec3f(0, 1, 1));
			if(sideBySide) viewMatrix = getViewMatrix(flip3d?0:1,globals.player.fileType == OsciAvAudioPlayer::QUAD);
			mesh2.draw(viewMatrix);
		}
		

		fbo.end();
	}
	
	ofSetColor(255);
	fbo.draw(0,0);
	
	if( exporting >= 2 ){
		string filename = ofToDataPath(exportDir + "/" + ofToString(exportFrameNum, 5, '0') + ".tiff");
		ofPixels pixels;
		fbo.readToPixels(pixels);
		ofSaveImage(pixels, filename);
	}
	
	if( showInfo || exporting > 0 ){
		ofSetColor(exporting>0?255:100);
		ofDrawBitmapString("Dropped: " + ofToString(dropped), 10, 20 );
		ofDrawBitmapString("FPS:     " + ofToString(ofGetFrameRate(),0), 10, 40 );
		ofDrawBitmapString("T: " + ofxFormatTime(globals.player.getPositionMS() / 1000.0), 10, 60); 
		if( exporting > 0 ){
			unsigned long long totalFrames = 1+globals.player.duration*globals.exportFrameRate/1000;
			int pct = exportFrameNum*100/totalFrames;
			ofDrawBitmapString("Format:  " + ofToString(fbo.getWidth()) + " x " + ofToString(fbo.getHeight()) + " @ " + ofToString(globals.exportFrameRate) + "fps (change in " + ofxToReadWriteableDataPath(")settings.txt") + ")", 10, 80);
			ofDrawBitmapString("Export:  " + ofToString(pct) + "%  (" + ofToString(exportFrameNum) + "/" + ofToString(totalFrames) + ")", 10, 100 );
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
	
	float waitBefore = 1;
	float extra = 0.2;
	float remainingTime = -(lastUpdateTime/1000.0f)+globals.secondsBeforeHidingMenu+lastMouseMoved/1000.0f;
	bool hovering = osciView->isMouseOver() || playlist->isMouseOver();
	if(remainingTime>-extra && !hovering){
		float t = ofMap(remainingTime, -extra, MAX(0.1,globals.secondsBeforeHidingMenu-waitBefore), 1, 0);
		if(t<0 || t>1){
			// nothing to draw
		}
		else{
			float T0 = -extra;
			float T1 = 0;
			float T2 = globals.secondsBeforeHidingMenu;

			t = MIN(t,1);
			float alpha = ofMap(remainingTime,T1,T2,1,0,true)*pow(ofMap(remainingTime,-extra,0.2,0,1,true),2);
			float w = ofMap(t,0.2,1,0,1,true); 
			ofSetColor(255, 255*alpha);
			ofPushMatrix();
			float s = mui::MuiConfig::scaleFactor;
			float W = ofGetWidth();
			float H = ofGetHeight();
			float mx = ofClamp(ofGetMouseX(), 0, W);
			float my = ofClamp(ofGetMouseY(), 0, W);
			float magicX = ofMap(mx,0,W,+1,-1,true);
			float magicY = ofMap(my,0,H,+1,-1,true);
			ofVec2f v = ofVec2f(magicX,magicY).getNormalized()*s*50;
			
			ofPath path;
			float size = ofMap(sqrt(t),0,0.2,0,1,true)*ofMap(t,0,1,30,3,true);
			ofTranslate(mx, my);
			ofScale(mui::MuiConfig::scaleFactor, mui::MuiConfig::scaleFactor);
			ofRotateZ(atan2(magicY,magicX)*RAD_TO_DEG);
			path.arc({0,0}, size, size, ofMap(t,0.5,1,0,359,true), ofMap(t,0,0.5,1,360,true));
			path.close();
			path.setFillColor(ofColor(255,150*alpha));
//			path.setFilled(false);
//			path.setStrokeWidth(1);
			//			float alpha = ofMap(t,0,0.1,0,1,true)*ofMap(t,0.9,1,1,0,true);
//			path.setStrokeColor(ofColor(255,alpha*255));
			path.draw();

			ofSetColor(255);
			ofPopMatrix();
		}
	}
}

void ofApp::exit(){
	stopApplication();
	ofstream out(ofxToReadWriteableDataPath("playlist.txt"));
	playlist->save(out);
	out.close();
	std::exit(0);
}


//----------------------------------------------------------	----
void ofApp::keyPressed  (int key){
	key = std::tolower(key);
	
	if( key == '\t' && !configView->isVisibleOnScreen()){
		osciView->visible = !osciView->visible;
		if( osciView->visible ){
			lastMouseMoved = ofGetElapsedTimeMillis();
		}
		else{
			lastMouseMoved = 0;
			mousePosBeforeHiding.x = ofGetMouseX();
			mousePosBeforeHiding.y = ofGetMouseY();
		}
	}

	if( key == 'f' || (ofGetKeyPressed(OF_KEY_ALT) && key == OF_KEY_RETURN) || key == OF_KEY_F11 ){
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
	
	if( key == 'p' ){
		playlistEnable ^= true;
	}
	
	if( ofGetKeyPressed(MUI_KEY_ACTION) && key == ',' ){
		ofSystemAlertDialog("All settings are in the editable file '" + ofxToReadWriteableDataPath("settings.txt") + "' \n"
							"\n"
							"1. Edit settings.txt\n"
							"2. Switch back to the oscilloscope application"
							);
		ofxNative::showFile(ofxToReadWriteableDataPath("settings.txt"));
	}
	if( ofGetKeyPressed(MUI_KEY_ACTION) && key == 'r'){
		globals.loadFromFile();
		osciView->fromGlobals();
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
	if (w == 0 || h == 0) return; 
	osciView->width = min(500.0f,w/mui::MuiConfig::scaleFactor);
	osciView->layout();
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
		bool res = globals.player.loadSound(fileToLoad);
		if(!res){
			currentFilename = fileToLoad;
			fileToLoad = "";
			globals.player.onEnd();
		}
		else{
			osciView->timeStretchSlider->slider->value = 1.0;
			nextWindowTitle = fileToLoad; // back to ui thread ^^
			currentFilename = fileToLoad;
			fileToLoad = "";
		}
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
		
		auto devs = micStream.getDeviceList();
		if(globals.micDeviceId<devs.size()){
			nextWindowTitle = devs[globals.micDeviceId].name + " @ " + ofToString(globals.sampleRate) + "Hz";
		}
	}
	else if( msg.message == "stop-mic" ){
		stopMic();
	}
	else if( msg.message.substr(0,5) == "load:" ){
		stopMic();
		fileToLoad = msg.message.substr(5);
	}
	else if(msg.message.substr(0,8) == "load-id:"){
		size_t itemId = atoll(msg.message.substr(8).c_str());
		fileToLoad = playlist->getItemPath(itemId);
		globals.currentlyPlayingItem = itemId; 
	}
	else if( msg.message == "next-timecode-style"){
		
	}
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	if( dragInfo.files.size() >= 1 ){
		// this runs on a separate thread.
		// we have to be careful not to make a mess!
		stopMic();
		playlistEnable = false;
		fileToLoad = dragInfo.files[0];
	}
}

//--------------------------------------------------------------
void ofApp::stopMic(){
	if (globals.micActive) {
		micStream.stop();
		micStream = ofSoundStream();
		globals.micActive = false;
		nextWindowTitle = currentFilename; // back to the previous window title
	}
}

//--------------------------------------------------------------
ofMatrix4x4 ofApp::getViewMatrix(int i, bool isQuad) {

	float gw = fbo.getWidth();
	float gh = fbo.getHeight();
	float w = gw*(osciView->sideBySide->selected && isQuad? 0.5 : 1);
	float h = gh;
	float aspectRatio = float(gw) / float(gh);
	float scale = min(w / gw, h / gh)*globals.scale;

	float dx = 0;
	if (osciView->sideBySide->selected && isQuad) {
		dx = i == 0 ? -0.5 : 0.5;
	}

	if (aspectRatio < 1.0) scale *= aspectRatio;
	ofMatrix4x4 result = ofMatrix4x4(
		scale/aspectRatio*(osciView->sideBySide->selected?0.5:1), 0.0, 0.0, 0.0,
		0.0, -scale, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		dx, 1.0 - h/gh, 0.0, 1.0);

	if (globals.invertX) result(0,0) *= -1;
	if (globals.invertY) result(1,1) *= -1;
	
	if (globals.flipXY) {
		result = ofMatrix4x4(
								 0.0, 1.0, 0.0, 0.0,
								 1.0, 0.0, 0.0, 0.0,
								 0.0, 0.0, 1.0, 0.0,
								 0.0, 0.0, 0.0, 1.0 ) * result;
	}
	
	return result;
}