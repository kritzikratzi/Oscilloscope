#define MINIAUDIO_IMPLEMENTATION
#define MA_COINIT_VALUE COINIT_APARTMENTTHREADED
#ifdef __APPLE__
	#define MA_NO_RUNTIME_LINKING
#endif

#include "util/miniaudio.h"
#undef MINIAUDIO_IMPLEMENTATION

#ifdef __APPLE__
#if MA_MAX_CHANNELS != 128
#error OSX Configuration should be set to 128ch to support blackhole64ch
#endif
#endif


#include "ofApp.h"
#include "globals.h"
#include "version.h"
#include <cctype> 
#include "ofxNative.h"
#include "ui/ExportScreen.h"
#include "MuiL.h"
#include "util/WickedLasers.h"

#include "GLFW/glfw3.h"

//--------------------------------------------------------------
void ofApp::setup(){
	globals.init();
	OsciMesh::init();
	MA_ZERO_OBJECT(&playDevice);
	MA_ZERO_OBJECT(&micDevice);

	texSender.setup("Oscilloscope");
	mui::MuiConfig::fontSize = 16;
	showInfo = false;
	dropped = 0;
	changed = false;
	clearFbos = false;
	lastMouseMoved = 0;
	exporting = 0;
	tooltipStyle.fontSize = 10;
	tooltipStyle.color = ofColor(255);
	
	applicationRunning = false; 
	ofSetVerticalSync(true);
	ofBackground(0);
	ofSetBackgroundAuto(true);
	
	ofSetFrameRate(60);
	
	root = new mui::Root();
	
	globals.player.loadSound(ofxToReadonlyDataPath("konichiwa.wav"));
//	globals.player.loadSound(ofxToReadonlyDataPath("c:\\Users\\hansi\\Desktop\\3dbounce.wav"));
	globals.player.setLoop(false);
	globals.player.stop();
	globals.player.onEnd = [&](){
		lock_guard<mutex> lock(mainThreadMutex);
		mainThreadTasks.push([&](){playlistItemEnded();});
	};
	
	configView = new ConfigView();
	configView->fromGlobals();
	root->add( configView );
	
	exportScreen = new ExportScreen();
	exportScreen->visible = false;
	root->add( exportScreen );
	
	playerOverlay = new PlayerOverlay();
	playerOverlay->visible = false;
	playerOverlay->filenameLabel->setText("");
	root->add( playerOverlay );
	
	playlist = new Playlist();
	root->add(playlist);
	{
		ifstream in(ofxToReadWriteableDataPath("playlist.txt"));
		playlist->load(in);
		in.close();
	}
	
	
	micLeft.loop = false;
	micRight.loop = false;
	micZMod.loop = false;

	initComplete = true; 
	startApplication();
	windowResized(ofGetWidth(), ofGetHeight());

	root->onDrawAbove.add([&]() {drawAbove(); });
	
	ofRegisterURLNotification(this);
	ofLoadURLAsync("https://www.oscilloscopemusic.com/version_oscilloscope.xml");
}


void ofApp::startApplication(){
	if (!initComplete) return; 
	if (globals.micActive) return; 
	if( applicationRunning ) return;
	applicationRunning = true;
	cout << "starting ..." << endl; 
	micLeft.playFrom(0);
	micRight.playFrom(0);
	micZMod.playFrom(0);

	playerOverlay->fromGlobals();
	playerOverlay->visible = true;
	forceHidden = false;
	playerOverlay->filenameLabel->setText(ofFile(globals.player.getFilename(),ofFile::Reference).getFileName());
	/*if( globals.autoDetect ){
		cout << "Running auto-detect for sound cards" << endl;
		getDefaultRtOutputParams( globals.deviceId, globals.sampleRate, globals.bufferSize, globals.numBuffers );
	}*/
	
	//if you want to set the device id to be different than the default
	cout << "Opening Sound Card: " << endl;
	cout << "    Name: " << globals.out_requested.name << endl;

	playDeviceConfig = ma_device_config_init(ma_device_type_playback);
	playDeviceConfig.playback.format = ma_format_f32;
	playDeviceConfig.playback.channels = 2;
	playDeviceConfig.periodSizeInFrames = 0;
	playDeviceConfig.sampleRate = 0;
	
	playDeviceConfig.dataCallback = [](ma_device* device, void* pOutput, const void* pInput, ma_uint32 frameCount) {
		ofApp * app = (ofApp*)device->pUserData;
		globals.out_actual.sampleRate = device->sampleRate;
		globals.out_actual.bufferSize = frameCount;
		globals.out_actual.name = device->playback.name;

		app->updateSampleRate();
		app->audioOut((float*)pOutput, frameCount, device->playback.channels);
	};

	playDeviceConfig.stopCallback = [](ma_device* device) {
		ofApp * app = (ofApp*)device->pUserData; 
		lock_guard<mutex> lock(app->mainThreadMutex);
		app->mainThreadTasks.push([app]() {app->startApplication(); });

	};
	playDeviceConfig.pUserData = this;
	//playDeviceConfig.playback.pDeviceID = globals.deviceId;

	do{ // just "do" to be able to break
		ma_context & context = *globals.context;
		ma_device_info* pPlaybackDeviceInfos;
		ma_uint32 playbackDeviceCount;
		ma_device_info* pCaptureDeviceInfos;
		ma_uint32 captureDeviceCount;
		ma_uint32 iDevice;
		ma_result result;

		result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
		if (result != MA_SUCCESS) {
			printf("Failed to retrieve device information.\n");
		}


		for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
			ma_device_info dev = pPlaybackDeviceInfos[iDevice];
			if (dev.name == globals.out_requested.name) {
				//it's a tiny leak, but it works. we accept it as it is, give it a little hug, and move on.
				playDeviceConfig.playback.pDeviceID = new ma_device_id();
				memcpy((void*)playDeviceConfig.playback.pDeviceID, &dev.id, sizeof(ma_device_id));
			}
		}

	} while (false);


	if (ma_device_init(NULL, &playDeviceConfig, &playDevice) != MA_SUCCESS) {
		printf("Failed to open playback device.\n");
		stopApplication(); 
	}

	if (ma_device_start(&playDevice) != MA_SUCCESS) {
		printf("Failed to start playback device.\n");
		stopApplication(); 
	}

	lastMouseMoved = ofGetElapsedTimeMillis();
}


void ofApp::stopApplication(){
	if( !applicationRunning ) return;
	globals.saveToFile();
	applicationRunning = false;
	ma_device_stop(&playDevice);
	ma_device_uninit(&playDevice);
	ma_device_uninit(&micDevice); 

	globals.micActive = false;
}



//--------------------------------------------------------------
void ofApp::update(){
	lastUpdateTime = ofGetElapsedTimeMillis();
	if( nextWindowTitle != "" ){
		setWindowRepresentedFilename(nextWindowTitle);
		nextWindowTitle = ""; 
	}
	
	configView->visible = playerOverlay->visible && playerOverlay->configButton->selected; 
	playlist->visible = playerOverlay->visible && playlistEnable;

	// process tasks
	{
		lock_guard<mutex> lock(mainThreadMutex);
		while(mainThreadTasks.size()>0){
			mainThreadTasks.front()();
			mainThreadTasks.pop();
		}
	}
	
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
				playerOverlay->fromGlobals();
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
		auto c = MUI_ROOT->findChildAt(muiGetMouseX(), muiGetMouseY()); 
		bool hovering = c != MUI_ROOT;
		if( !hovering ){
			playerOverlay->visible = false;
			mousePosBeforeHiding.x = ofGetMouseX();
			mousePosBeforeHiding.y = ofGetMouseY();
		}
	}

	if( !applicationRunning && !globals.micActive){
		ofShowCursor();
		return;
	}

	ofVec2f mousePos(ofGetMouseX(), ofGetMouseY());
	bool insideWindow = ofRectangle(0,0,ofGetWidth(),ofGetHeight()).inside(mousePos);
	if( !forceHidden && lastUpdateTime-lastMouseMoved < globals.secondsBeforeHidingMenu*1000 && playerOverlay->visible == false ){
		bool movedEnough = mousePos.distance(mousePosBeforeHiding) > 10*mui::MuiConfig::scaleFactor;
		if( insideWindow && movedEnough ){
			// okay, we moved enough!
			playerOverlay->visible = true;
		}
	}

	if( playerOverlay->visible || !insideWindow) ofShowCursor();
	else ofHideCursor();
	
	// center the ui
	if (playerOverlay->sideBySideToggle->selected && globals.player.fileType == OsciAvAudioPlayer::QUAD ) {
		playerOverlay->x = ofGetWidth() / mui::MuiConfig::scaleFactor * 1 / 4.0f - playerOverlay->width / 2;
	}
	else {
		playerOverlay->x = ofGetWidth() / mui::MuiConfig::scaleFactor / 2 - playerOverlay->width / 2;
	}
	playerOverlay->y = ofGetHeight() / mui::MuiConfig::scaleFactor - playerOverlay->height - 60;

	mui::L(configView).above(playerOverlay, 5);

	// layout playlist
	{
		playlist->x = playerOverlay->x + playerOverlay->width + 5;
		playlist->y = playerOverlay->y;
		float w = max(10.0f, muiGetWidth() - playlist->x - 30);
		if (playlist->width != w) {
			playlist->width = w; 
			playlist->height = playerOverlay->height; 
			playlist->handleLayout(); 
		}
	}

	/////////////////////////////////////////////////
	
	// are we not export?
	if( exporting == 0 ){
		updateSampleRate();
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
			if(video_writer.isOpen()){
				video_writer.addAudioFrame(output, len);
			}
		}
		while( globals.player.getPositionMS() < targetTimeMS && len > 0 );
		
		if( len == 0 ){
			// save this frame, then end it!
			exporting = 3;
		}
		
	}
	else if( exporting == 3 ){
		if(video_writer.isOpen()){
   			video_writer.close();
		}
		
	
		exporting = 0;
		exportSampleRate = 0;
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

	MonoSample &left = globals.micActive?micLeft:globals.player.left192;
	MonoSample &right = globals.micActive?micRight:globals.player.right192;
	MonoSample &zMod = globals.micActive?micZMod:globals.player.zMod192;
	
	left.playFrom(0);
	right.playFrom(0);
	zMod.playFrom(0);
	
	bool isMono =
		(!globals.micActive && globals.player.fileType == OsciAvAudioPlayer::MONO) ||
		(globals.micActive && micChannels == 1);
	bool isZModulated =
		(!globals.micActive && globals.player.fileType == OsciAvAudioPlayer::STEREO_ZMODULATED) ||
		(globals.micActive && micChannels >= 3);
	bool isQuad = !globals.micActive && globals.player.fileType == OsciAvAudioPlayer::QUAD;
	
	playerOverlay->sideBySideToggle->visible = isQuad;
	playerOverlay->flip3dToggle->visible = isQuad;
	playerOverlay->zModulationToggle->visible = isZModulated;
	
	
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
		
		bool sideBySide = playerOverlay->sideBySideToggle->selected && globals.player.fileType == OsciAvAudioPlayer::QUAD;
		bool flip3d = playerOverlay->flip3dToggle->selected;
		ofMatrix4x4 viewMatrix = getViewMatrix(flip3d?1:0,globals.player.fileType == OsciAvAudioPlayer::QUAD);
		ofFloatColor color = ofFloatColor::fromHsb(globals.hue/360.0, 1, 1);

		float intensityScale = (globals.micActive ? 2 * 192000.0/max(8000,globals.in_actual.sampleRate) : 1);

		mesh.uSize = globals.strokeWeight/1000.0;
		mesh.uIntensityBase = max(0.0f,mesh.uIntensity-0.4f)*0.7f-1000.0f*mesh.uSize/500.0f;
		mesh.uIntensity = globals.intensity/sqrtf(globals.timeStretch)*intensityScale;
		mesh.uHue = globals.hue;
		if (globals.player.fileType == OsciAvAudioPlayer::QUAD && !sideBySide) {
			mesh.uRgb = (flip3d ? ofVec3f(0, 1, 1) : ofVec3f(1, 0, 0));
		}
		else {
			if (globals.hue == 360) {
				mesh.uRgb = ofVec3f(1, 1, 1);
			}
			else {
				ofFloatColor col = ofFloatColor::fromHsb(globals.hue / 360.0, 1, 1);
				mesh.uRgb = ofVec3f(col.r, col.g, col.b);
			}
		};
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
	texSender.update(fbo.getTexture());

	if( exporting >= 2 ){
		ofPixels pixels;
		fbo.readToPixels(pixels);
		if(video_writer.isOpen()){
			video_writer.addVideoFrame(pixels);
		}
		else{
			string filename	= ofToDataPath(exportDir + "/" + ofToString(exportFrameNum, 5, '0') + "." + exportExt);
			ofSaveImage(pixels, filename);
		}
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
}

//--------------------------------------------------------------
void ofApp::drawAbove() {
	auto pos = muiGetMousePos();
	auto thing = root->findChildAt(pos.x, pos.y, true, true); 
	if (thing) {
		auto text = thing->getPropertyString("tooltip"); 
		if (text != "") {
			auto container_bb = thing->getGlobalBounds(); 
			auto text_bb = mui::Helpers::getFontStash().getTextBounds(text, tooltipStyle, 0, 0); 
			//ofRectangle dest(container_bb.x, container_bb.y - text_bb.height- 3, text_bb.width + 6, text_bb.height + 2);
			ofRectangle dest(0, root->height - text_bb.height - 3, text_bb.width + 6, text_bb.height + 2);
			ofSetColor(0);
			ofDrawRectangle(dest);
			mui::Helpers::getFontStash().draw(text, tooltipStyle, dest.x+3, dest.y+text_bb.height+1);
			ofSetColor(255);
		}
	}
}


void ofApp::exit(){
	stopMic(); 
	stopApplication();
	ofstream out(ofxToReadWriteableDataPath("playlist.txt"));
	playlist->save(out);
	out.close();
	globals.laserPtr->die();
}


//----------------------------------------------------------	----
void ofApp::keyPressed  (int key){
	if(key < 255) key = std::tolower(key);
	
	if( key == '\t'){
		playerOverlay->visible = !playerOverlay->visible;
		if( playerOverlay->visible ){
			forceHidden = false;
			lastMouseMoved = ofGetElapsedTimeMillis();
		}
		else{
			forceHidden = true;
			lastMouseMoved = 0;
			mousePosBeforeHiding.x = ofGetMouseX();
			mousePosBeforeHiding.y = ofGetMouseY();
		}
	}

	if( key == 'f' || (ofGetKeyPressed(OF_KEY_ALT) && key == OF_KEY_RETURN) || key == OF_KEY_F11 ){
		// nasty!
		playerOverlay->fullscreenToggle->clickAndNotify();
	}
	
	if( key == OF_KEY_ESC ){
		playerOverlay->fullscreenToggle->clickAndNotify(false);
	}
	
	if( key == ' '  ){
		playerOverlay->playButton->clickAndNotify();
	}
	
	if( key == 'r' ){
		clearFbos = true;
	}
	
	if( key == 'i' ){
		showInfo ^= true;
	}
	
	if( key == 'e' && exporting == 0 ){
		if(globals.micActive){
			ofSystemAlertDialog("Cannot export when microphone is active");
		}
		else if(exportScreen->visible){
			exportScreen->visible = false;
		}
		else{
			globals.player.stop();
			exportScreen->show(ofFile(globals.player.getFilename(),ofFile::Reference));
			exportScreen->setBounds(0,0,MUI_ROOT->width, MUI_ROOT->height);
			exportScreen->toFront();
		}
		return;
	}
	
	if( key == 'p' ){
		gotMessage(ofMessage("toggle-playlist"));
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
		playerOverlay->fromGlobals();
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
	playerOverlay->width = min(500.0f,w/mui::MuiConfig::scaleFactor);
	playerOverlay->layout();
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
	if( globals.micActive ){
		switch(nChannels){
			case 0: 
				break; 
			case 1:
				micLeft.append(input, bufferSize,1);
				micRight.append(input,bufferSize,1);
				break;
			case 2:
				micLeft.append(input, bufferSize,2);
				micRight.append(input+1,bufferSize,2);
				break;
			case 3:
				micLeft.append(input, bufferSize,3);
				micRight.append(input+1,bufferSize,3);
				micZMod.append(input+2,bufferSize,3);
				break;
			default: // wtf :D
				micLeft.append(input, bufferSize,nChannels);
				micRight.append(input+1,bufferSize,nChannels);
				micZMod.append(input + 2, bufferSize, nChannels);
				break;
		}
	}

	globals.laserPtr->addBuffer(input, bufferSize, nChannels, globals.in_actual.sampleRate);
}

void ofApp::audioOut( float * output, int bufferSize, int nChannels ){
	if( fileToLoad != "" ){
		globals.timeStretch = 1.0;
		bool res = globals.player.loadSound(fileToLoad);
		string fname = ofFile(fileToLoad,ofFile::Reference).getFileName();
		if(!res){
			playerOverlay->filenameLabel->setText("Failed to load " + fname);
			currentFilename = fileToLoad;
			fileToLoad = "";
			playlistItemEnded(); 
		}
		else{
			playerOverlay->filenameLabel->setText(fname);
			playerOverlay->timeStretchSlider->slider->value = 1.0;
			nextWindowTitle = fileToLoad; // back to ui thread ^^
			currentFilename = fileToLoad;
			fileToLoad = "";
		}
	}
	
	memset(output, 0, bufferSize*nChannels);
	if( globals.player.isLoaded && exporting == 0 && !globals.micActive ){
		globals.player.audioOut(output, bufferSize, nChannels);
		if (globals.player.isPlaying) {
			globals.laserPtr->addBuffer(output, bufferSize, nChannels, globals.out_actual.sampleRate);
		}
		AudioAlgo::scale(output, globals.outputVolume, nChannels*bufferSize);
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
	if (msg.message == "config-pressed") {
		// nothing to do, hurray :) see ::update()
	}
	else if (msg.message == "load-pressed"){
		auto menu = new FMenu<string>(0, 0, 300, 0);
		menu->opaque = true;
		menu->onAfterRemove.add([](mui::Container * menu, mui::Container * parent) {
			MUI_ROOT->safeDelete(menu);
		});
		
		menu->addOption("Open file", "open-file", [menu,this]() {
			ofFileDialogResult res = ofSystemLoadDialog("Load audio file", false );
			if (res.bSuccess) {
				playlist->addFile(ofFile(res.filePath, ofFile::ReadOnly));
			}
			MUI_ROOT->removePopup(menu);
			playlist->handleLayout();
		});
		menu->addOption("Open folder", "open-folder", [menu,this]() {
			auto res = ofSystemLoadDialog("Add Folder", true);
			if (res.bSuccess) {
				playlist->addFile(ofFile(res.filePath, ofFile::ReadOnly));
			}
			MUI_ROOT->removePopup(menu);
			playlist->handleLayout();
		});
		
		MUI_ROOT->showPopupMenu(menu, root, muiGetMouseX(), muiGetMouseY(), mui::Left, mui::Bottom);

	}
	else if (msg.message == "out-choice-changed") {
		configView->toGlobals(); 
		stopApplication(); 
		startApplication(); 
	}
	else if( msg.message == "start-mic" ){
		if( exporting != 0 ) return;
		globals.player.stop();
		
		startMic(); 
	}
	else if( msg.message == "stop-mic" ){
		stopMic();
		startApplication(); 
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
	else if( msg.message == "begin-export"){
		beginExport(ofFile(exportScreen->getFile(),ofFile::Reference));
	}
	else if (msg.message == "toggle-playlist") {
		playlistEnable ^= true; 
		playerOverlay->showPlaylistToggle->selected = playlistEnable;
	}
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	playlist->fileDragged(dragInfo);
}

//--------------------------------------------------------------
void ofApp::beginExport(const ofFile & file){
	if(exporting) return;
	
	exportFormat = globals.exportFormat;
	exportDir = file.getAbsolutePath();
	
	switch(exportFormat){
		case ExportFormat::H264:
			exportExt = "mp4";
			exportSampleRate = 96000;
			break;
		case ExportFormat::IMAGE_SEQUENCE_TIFF:
			exportExt = "tiff";
			exportSampleRate = 96000;
			break;
		case ExportFormat::IMAGE_SEQUENCE_PNG:
			exportExt = "png"; break;
			exportSampleRate = 96000;
	}


	if(exportFormat == ExportFormat::H264){
		// set up video_writer
		OsciVideoWriter::OutputConfig output_config = {
			.video_framerate = globals.exportFrameRate,
			.video_width = globals.exportWidth,
			.video_height = globals.exportHeight,
			.audio_sample_rate = exportSampleRate,
			.audio_n_channels = 2
		};
		if(!video_writer.open(exportDir, output_config)){
			ofSystemAlertDialog("Something went wrong while trying to create export mp4 file. Sorry?");
			return;
		}
	}
	else{
		ofDirectory dir(exportDir);
		dir.create();
		if( !dir.exists() || !dir.isDirectory() ){
			// don't export!
			ofSystemAlertDialog("Something went wrong while trying to create export directory. Sorry?");
			return;
		}
	}
	
	exporting = 1;
}

//--------------------------------------------------------------
void ofApp::playlistItemEnded(){
	// see if we have a "next" item
	if(exporting){
		// do nothing, let it end.
	}
	else{
		auto next = playlist->getNextItemInPlaylist(globals.currentlyPlayingItem);
		if(next.id > 0){
			fileToLoad = next.filename;
			globals.currentlyPlayingItem = next.id;
		}
		else if(globals.currentlyPlayingItem == 0){
			globals.player.setPositionMS(0);
		}
		else{
			globals.currentlyPlayingItem = 0;
		}
	}
}

void ofApp::updateSampleRate(){
	double s = 1;
	if(globals.analogMode == 0) s = 1;
	else s = 4;
	
	int64_t rate = max((int64_t)globals.out_actual.sampleRate/4,(int64_t)std::round(globals.out_actual.sampleRate*globals.timeStretch));
	
	double vrate =
		(globals.analogMode!=0?globals.out_actual.sampleRate : globals.player.getFileSampleRate())
		* s * globals.timeStretch;
	
	if(exportSampleRate !=0){
		rate = exportSampleRate;
		vrate =
			(globals.analogMode!=0?rate : globals.player.getFileSampleRate())
			* s;
	}

	globals.player.setupAudioOut(2, rate, globals.analogMode!=0, max(vrate,192000.0));
}


//--------------------------------------------------------------
void ofApp::startMic() {
	stopMic(); 
	stopApplication();
	
	micLeft.clear();
	micRight.clear();
	micZMod.clear();

	auto info = playerOverlay->getSelectedMicDeviceInfo();
	micChannels = info.info.maxChannels;
	micDeviceConfig = ma_device_config_init(info.type);
	micDeviceConfig.capture.format = ma_format_f32;
	micDeviceConfig.capture.channels = 0;
	micDeviceConfig.periodSizeInFrames = 0;
	micDeviceConfig.sampleRate = 0;
	micDeviceConfig.dataCallback = [](ma_device* device, void* pOutput, const void* pInput, ma_uint32 frameCount) {
		ofApp * app = (ofApp*)device->pUserData;
		app->micChannels = device->capture.channels;
		globals.in_actual.sampleRate = device->sampleRate;
		globals.in_actual.bufferSize = frameCount;
		globals.in_actual.name = device->playback.name;

		app->audioIn((float*)pInput, frameCount, device->capture.channels);
	};

	micDeviceConfig.stopCallback = [](ma_device* device) {
		ofApp * app = (ofApp*)device->pUserData;
		lock_guard<mutex> lock(app->mainThreadMutex);
		if (globals.micActive) {
			app->mainThreadTasks.push([app]() {app->startMic(); });
		}
	};
	micDeviceConfig.pUserData = this;
	//playDeviceConfig.playback.pDeviceID = globals.deviceId;

	if (strcmp(info.info.name,"") != 0) {
		if (!micDeviceConfig.capture.pDeviceID) micDeviceConfig.capture.pDeviceID = new ma_device_id();
		memcpy((void*)micDeviceConfig.capture.pDeviceID, &info.info.id, sizeof(ma_device_id));

	}

	if (ma_device_init(NULL, &micDeviceConfig, &micDevice) != MA_SUCCESS) {
		printf("Failed to open capture device.\n");
		stopMic();
		return;
	}

	if (ma_device_start(&micDevice) != MA_SUCCESS) {
		printf("Failed to start capture device.\n");
		stopMic();
		return; 
	}


	globals.micActive = true;
	playerOverlay->filenameLabel->setText(string(micDevice.capture.name));
	lastMouseMoved = ofGetElapsedTimeMillis();

}
//--------------------------------------------------------------
void ofApp::stopMic(){
	if (globals.micActive) {
		globals.micActive = false;
		ma_device_uninit(&micDevice);
		nextWindowTitle = currentFilename; // back to the previous window title*/
	}
}

//--------------------------------------------------------------
ofMatrix4x4 ofApp::getViewMatrix(int i, bool isQuad) {

	float gw = fbo.getWidth();
	float gh = fbo.getHeight();
	float w = gw*(playerOverlay->sideBySideToggle->selected && isQuad? 0.5 : 1);
	float h = gh;
	float aspectRatio = float(gw) / float(gh);
	float scale = min(w / gw, h / gh)*globals.scale;

	float dx = 0;
	if (playerOverlay->sideBySideToggle->selected && isQuad) {
		dx = i == 0 ? -0.5 : 0.5;
	}

	if (aspectRatio < 1.0) scale *= aspectRatio;
	ofMatrix4x4 result = ofMatrix4x4(
		scale/aspectRatio*((isQuad && playerOverlay->sideBySideToggle->selected)?0.5:1), 0.0, 0.0, 0.0,
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

void ofApp::urlResponse(ofHttpResponse & response){
	ofStopURLLoader();

	ofXml xml;
	xml.load(response.data);
	
	// verA<verB?-1   verA=verB?0   verA>verB?1
	auto compare = []( string verA, string verB ){
		auto va = ofSplitString(verA, ".");
		auto vb = ofSplitString(verB, ".");
		size_t N = min(va.size(), vb.size());
		for( size_t i = 0; i < N; i++){
			int a = ofToInt(va[i]);
			int b = ofToInt(vb[i]);
			if( a < b) return -1;
			else if( a > b ) return 1;
		}
		return va.size()<vb.size()?-1:(va.size()==vb.size()?0:1);
	};
	
	string latestVersionText = "";
	string latestVersion = app_version;
	bool foundUpdate = false;
	string websiteUrl = xml.getChild("items").getAttribute("website").getValue();
	
	auto items = xml.getChild("items").getChildren("item");
	for(auto & item : items){
		if(compare(latestVersion,item.getChild("version").getValue()) == -1){
			latestVersionText = item.getChild("title").getValue();
			latestVersion = item.getChild("version").getValue();
			foundUpdate = true;
		}
	}
	
	if(foundUpdate){
		//regex pattern(R"(^(?:(?:https?|ftp):\/\/)(?:\S+(?::\S*)?@)?(?:(?!(?:10|127)(?:\.\d{1,3}){3})(?!(?:169\.254|192\.168)(?:\.\d{1,3}){2})(?!172\.(?:1[6-9]|2\d|3[0-1])(?:\.\d{1,3}){2})(?:[1-9]\d?|1\d\d|2[01]\d|22[0-3])(?:\.(?:1?\d{1,2}|2[0-4]\d|25[0-5])){2}(?:\.(?:[1-9]\d?|1\d\d|2[0-4]\d|25[0-4]))|(?:(?:[a-z\u00a1-\uffff0-9]-*)*[a-z\u00a1-\uffff0-9]+)(?:\.(?:[a-z\u00a1-\uffff0-9]-*)*[a-z\u00a1-\uffff0-9]+)*(?:\.(?:[a-z\u00a1-\uffff]{2,}))\.?)(?::\d{2,5})?(?:[/?#]\S*)?$)", regex_constants::icase | regex_constants::ECMAScript );
		
		if(/*!regex_match(websiteUrl, pattern )*/ websiteUrl == ""){
			websiteUrl = "https://www.oscilloscopemusic.com";
		}
		
		
		//ofSystemAlertDialog("New version available: " + latestVersionText);
		playerOverlay->updateButton->setProperty<string>("url", websiteUrl);
		playerOverlay->updateButton->label->setText("Update available: From " + app_version + " to " + latestVersionText );
		playerOverlay->updateButton->visible = true;
		playerOverlay->handleLayout();
		playlist->height = playerOverlay->height;
		playlist->handleLayout();
	}
}
