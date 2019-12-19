#include "PlayerOverlay.h"
#include "ofxFontAwesome.h"
#include "ofApp.h"
#include "MuiL.h"

static string timestring(double t);

PlayerOverlay::PlayerOverlay( float x_, float y_, float width_, float height_)
: mui::Container( x_, y_, width_, height_ ){
	float x = 10, y = 10, w = 400, h = 30;
	bg = ofColor(0,0,255,255);
	opaque = true;

	configButton = new FaToggleButton( ofxFontAwesome::cogs, ofxFontAwesome::cogs, x, y, h, h );
	configButton->bg = ofColor(0,0);
	ofAddListener( configButton->onPress, this, &PlayerOverlay::buttonPressed );
	y += configButton->height + 10;
	add( configButton );
	
	scaleLabel = addLabel( "Scale" );
	scaleSlider = new mui::SliderWithLabel( x, y, w, h, 0.1, 5, 1, 2 );
	scaleSlider->label->fg = ofColor(255);
	ofAddListener( scaleSlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	y += scaleSlider->height + 10;
	add( scaleSlider );
	
	
	w = 90;
	flipXYToggle = new FaToggleButton( ofxFontAwesome::repeat, ofxFontAwesome::repeat, x, y, h, h );
	flipXYToggle->bg = ofColor(0,0);
	ofAddListener( flipXYToggle->onPress, this, &PlayerOverlay::buttonPressed );
	add( flipXYToggle);
	x += 100;
	
	invertXToggle = new FaToggleButton( ofxFontAwesome::arrows_h, ofxFontAwesome::arrows_h, x, y, h, h );
	invertXToggle->bg = ofColor(0,0);
	ofAddListener( invertXToggle->onPress, this, &PlayerOverlay::buttonPressed );
	add( invertXToggle);
	x += 100;
	
	invertYToggle = new FaToggleButton( ofxFontAwesome::arrows_v, ofxFontAwesome::arrows_v, x, y, h, h );
	invertYToggle->bg = ofColor(0,0);
	ofAddListener( invertYToggle->onPress, this, &PlayerOverlay::buttonPressed );
	add( invertYToggle);
	x += 100;
	
	filenameLabel = new mui::Label("-", x, y, w, h);
	add(filenameLabel);
	
	fullscreenToggle = new FaToggleButton( ofxFontAwesome::expand, ofxFontAwesome::compress, x, y, h, h );
	fullscreenToggle->bg = ofColor(0,0);
	ofAddListener( fullscreenToggle->onPress, this, &PlayerOverlay::buttonPressed );
	add( fullscreenToggle);
	
	loadFileButton = new FaButton( ofxFontAwesome::folder_open, x, y, h, h );
	loadFileButton->bg = ofColor(0,0);
	ofAddListener( loadFileButton->onPress, this, &PlayerOverlay::buttonPressed );
	add( loadFileButton );
	
	useMicButton = new FaToggleButton( ofxFontAwesome::microphone, ofxFontAwesome::microphone_slash, x, y, h, h );
	useMicButton->bg = ofColor(0,0);
	ofAddListener( useMicButton->onPress, this, &PlayerOverlay::buttonPressed );
	add( useMicButton );
	
	playButton = new FaToggleButton( ofxFontAwesome::play, ofxFontAwesome::pause, x, y, h, h );
	playButton->bg = ofColor(0,0);
	ofAddListener( playButton->onPress, this, &PlayerOverlay::buttonPressed );
	add( playButton );
	
	sideBySideToggle = new FaToggleButton(ofxFontAwesome::cube, ofxFontAwesome::cube, 10, 1, h, h);
	sideBySideToggle->bg = ofColor(0,0);
	ofAddListener(sideBySideToggle->onPress, this, &PlayerOverlay::buttonPressed);
	add(sideBySideToggle);

	flip3dToggle = new FaToggleButton(ofxFontAwesome::exchange, ofxFontAwesome::exchange, 10, 1, h, h);
	flip3dToggle->bg = ofColor(0,0);
	ofAddListener(flip3dToggle->onPress, this, &PlayerOverlay::buttonPressed);
	add(flip3dToggle);

	zModulationToggle = new FaToggleButton(ofxFontAwesome::adjust, ofxFontAwesome::adjust, 10, 1, h, h);
	zModulationToggle->bg = ofColor(0,0);
	ofAddListener(zModulationToggle->onPress, this, &PlayerOverlay::buttonPressed);
	add(zModulationToggle);

	showPlaylistToggle  = new FaToggleButton(ofxFontAwesome::list, ofxFontAwesome::list, 10, 1, h, h);
	showPlaylistToggle->bg = ofColor(0,0);
	ofAddListener(showPlaylistToggle->onPress, this, &PlayerOverlay::buttonPressed);
	add(showPlaylistToggle);
	
	analogModeToggle  = new mui::ToggleButton("A", 10, 1, h, h);
	analogModeToggle->fg = ofColor(255);
	analogModeToggle->selectedFg = ofColor(0);
	analogModeToggle->bg = ofColor(0,0);
	ofAddListener(analogModeToggle->onPress, this, &PlayerOverlay::buttonPressed);
	add(analogModeToggle);
	
	x = 10;
	y += invertYToggle->height + 10;
	
	timeSlider = new mui::Slider( 0, 0, w, h );
	add( timeSlider );
	
	timeLabelButton = new mui::Button("-00:00:00", 0,0, 30,30);
	timeLabelButton->fitWidthToLabel(3);
	ofAddListener(timeLabelButton->onPress, this, &PlayerOverlay::buttonPressed);
	add(timeLabelButton);
	
	outputVolumeLabel = addLabel( "Volume" );
	outputVolumeSlider = new mui::SliderWithLabel(0, 0, 100, h, 0, 1, 0.8, 2 );
	ofAddListener( outputVolumeSlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	outputVolumeSlider->label->fg = ofColor(255);
	add(outputVolumeSlider);
	
	strokeWeightLabel = addLabel( "Stroke Weight" );
	strokeWeightSlider = new mui::SliderWithLabel(0, 0, 100, h, 1, 50, 4, 1 );
	ofAddListener( strokeWeightSlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	strokeWeightSlider->label->fg = ofColor(255);
	add( strokeWeightSlider );
	
	timeStretchLabel = addLabel( "Speed" );
	timeStretchSlider = new mui::SliderWithLabel(0, 0, 100, h, 0.25, 100, 1, 2 );
	timeStretchSlider->slider->valueMapper = make_shared<mui::Slider::MapperLog>(6000);
	ofAddListener( timeStretchSlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	timeStretchSlider->label->fg = ofColor(255);
	add( timeStretchSlider );
	
	blurLabel = addLabel( "Blur" );
	blurSlider = new mui::SliderWithLabel(0,0, 100, h, 0, 255, 30, 0);
	ofAddListener( blurSlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	blurSlider->label->fg = ofColor(255);
	add(blurSlider);
	
	numPtsLabel = addLabel( "Points" );
	numPtsSlider = new mui::SliderWithLabel(0,0,100,h, 1, 100, 20, 0);
	ofAddListener( numPtsSlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	numPtsSlider->label->fg = ofColor(255);
	add(numPtsSlider);

	intensityLabel = addLabel( "Intensity" );
	intensitySlider = new mui::SliderWithLabel(0,0,100,h,0,1, 0.5, 2);
	ofAddListener( intensitySlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	intensitySlider->label->fg = ofColor(255);
	add(intensitySlider);
	
	afterglowLabel = addLabel( "Afterglow" );
	afterglowSlider = new mui::SliderWithLabel(0,0,100,h,0,1, 0.3, 2);
	ofAddListener( afterglowSlider->slider->onChange, this, &PlayerOverlay::sliderChanged );
	afterglowSlider->label->fg = ofColor(255);
	add(afterglowSlider);
	
	
	

	// make labels the same width
	outputVolumeSlider->label->width = timeStretchSlider->label->width;
	
	layout();
}

void PlayerOverlay::layout(){
	mui::L({analogModeToggle, configButton,fullscreenToggle, showPlaylistToggle}).columnsFromRight({width, 0},1);
	mui::L({zModulationToggle,flip3dToggle,sideBySideToggle }).columnsFromRight({configButton->x-10,0},1);

	mui::L(playButton).pos(10,40);
	mui::L(timeLabelButton).pos(width-10-timeLabelButton->width,playButton->y);
	mui::L(timeSlider).rightOf(playButton, 10).stretchToRightEdgeOfParent(width-(timeLabelButton->x-10));
	
	mui::L(loadFileButton).below(playButton, 10);
	mui::L(useMicButton).rightOf(loadFileButton, 10);
	mui::L(outputVolumeLabel).rightOf(useMicButton,20);
	mui::L(outputVolumeSlider).rightOf(outputVolumeLabel,5).stretchToRightEdgeOfParent(10);
	
	mui::L(timeStretchLabel).below(outputVolumeLabel).alignRightEdgeTo(outputVolumeLabel);
	mui::L(timeStretchSlider).rightOf(timeStretchLabel,5).stretchToRightEdgeOfParent(10);
	
	mui::L(invertXToggle).below(timeStretchLabel, 20).alignLeftEdgeTo(loadFileButton);
	mui::L(invertYToggle).rightOf(invertXToggle, 10);
	mui::L(flipXYToggle).rightOf(invertYToggle,10);
	mui::L(scaleLabel).rightOf(flipXYToggle,20);
	mui::L(scaleSlider).rightOf(scaleLabel,5).stretchToRightEdgeOfParent(10);
	
	mui::L(strokeWeightLabel).below(scaleLabel).alignRightEdgeTo(scaleLabel);
	mui::L(strokeWeightSlider).rightOf(strokeWeightLabel,5).stretchToRightEdgeOfParent(10);
	
	mui::L(filenameLabel).leftOf(configButton).stretchToLeftEdgeOfParent(10);
	
	/*mui::L(blurLabel).below(strokeWeightLabel).alignRightEdgeTo(strokeWeightLabel);
	mui::L(blurSlider).rightOf(blurLabel,5).stretchToRightEdgeOf(this,10);
	
	mui::L(numPtsLabel).below(blurLabel).alignRightEdgeTo(blurLabel);
	mui::L(numPtsSlider).rightOf(numPtsLabel,5).stretchToRightEdgeOf(this,10);*/
	blurLabel->visible = false;
	blurSlider->visible = false;
	numPtsLabel->visible = false;
	numPtsSlider->visible = false;
	
	mui::L(intensityLabel).below(strokeWeightLabel).alignRightEdgeTo(strokeWeightLabel);
	mui::L(intensitySlider).rightOf(intensityLabel,5).stretchToRightEdgeOfParent(10);
	
	mui::L(afterglowLabel).below(intensityLabel).alignRightEdgeTo(intensityLabel);
	mui::L(afterglowSlider).rightOf(afterglowLabel,5).stretchToRightEdgeOfParent(10);
	
	height = afterglowSlider->y + afterglowSlider->height + 10;
}


// tiny helper functions.
// these should somehow be in mui, they're so handy
// returns false if the user changed the value
bool updateSlider( mui::Slider * slider, float targetValue, float &lastValue ){
	if( slider->hasFocus() ){
		if( lastValue != slider->value ){
			lastValue = slider->value;
			return false;
		}
		else{
			return true;
		}
	}
	else{
		slider->value = targetValue;
		return true;
	}
	
}

//--------------------------------------------------------------
void PlayerOverlay::update(){
	static float lastTimeVal = -1;
	useMicButton->selected = globals.micActive;
	timeSlider->visible = !globals.micActive;
	outputVolumeLabel->visible = !globals.micActive;
	outputVolumeSlider->visible = !globals.micActive;
	playButton->visible = !globals.micActive;
	analogModeToggle->selected = globals.analogMode == 0?false:true; 
	
	if( !globals.micActive ){
		if( !updateSlider(timeSlider, globals.player.getPosition(), lastTimeVal ) ){
			globals.player.setPosition(timeSlider->value);
		}

		if( globals.player.isPlaying != playButton->selected ){
			playButton->selected = globals.player.isPlaying;
			playButton->commit();
		}
	}
	
	
	double currentTime = globals.player.getPositionMS()/1000.0;
	double duration = globals.player.duration/1000.0;
	
	if( timeLabelMode == 0 ){
		timeLabelButton->label->text = timestring(currentTime);
		timeLabelButton->bg = ofColor(0,0);
		timeLabelButton->label->fg = ofColor(255);
	}
	else if( timeLabelMode == 1 ){
		timeLabelButton->label->text = "-" + timestring(duration - currentTime);
		timeLabelButton->bg = ofColor(0,0);
		timeLabelButton->label->fg = ofColor(255);
	}
	timeLabelButton->label->commit();
}


//--------------------------------------------------------------
void PlayerOverlay::draw(){
	ofSetColor(150);
	ofDrawLine( 10, flipXYToggle->y - 10, width-10, flipXYToggle->y - 10 );
	ofSetColor(255);
}



//--------------------------------------------------------------
void PlayerOverlay::touchDown( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void PlayerOverlay::touchMoved( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void PlayerOverlay::touchUp( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void PlayerOverlay::touchDoubleTap( ofTouchEventArgs &touch ){
}


void PlayerOverlay::fromGlobals(){
	outputVolumeSlider->slider->value = globals.outputVolume;
	
	if( globals.player.isPlaying != playButton->selected ){
		playButton->selected = globals.player.isPlaying;
		playButton->commit();
	}
	
	scaleSlider->slider->value = globals.scale;
	strokeWeightSlider->slider->value = globals.strokeWeight;
	timeStretchSlider->slider->value = globals.timeStretch;
	blurSlider->slider->value = globals.blur;
	numPtsSlider->slider->value = globals.numPts;
	intensitySlider->slider->value = globals.intensity;
	afterglowSlider->slider->value = globals.afterglow;
	invertXToggle->selected = globals.invertX;
	invertXToggle->commit();
	invertYToggle->selected = globals.invertY;
	invertYToggle->commit();
	flipXYToggle->selected = globals.flipXY;
	flipXYToggle->commit();
	zModulationToggle->selected = globals.zModulation;
}

//--------------------------------------------------------------
void PlayerOverlay::buttonPressed( const void * sender, ofTouchEventArgs & args ){
	mui::Container * container = (mui::Container*) sender;
	
	if( sender == configButton ){
		ofBaseApp * app = ofGetAppPtr();
		app->gotMessage( ofMessage( "config-pressed" ) );
	}
	else if( sender == playButton ){
		if( globals.player.isPlaying ){
			globals.player.stop();
		}
		else{
			globals.player.play();
		}
	}
	else if( sender == invertXToggle){
		globals.invertX = invertXToggle->selected;
	}
	else if( sender == invertYToggle){
		globals.invertY = invertYToggle->selected;
	}
	else if( sender == flipXYToggle){
		globals.flipXY = flipXYToggle->selected;
	}
	else if( sender == zModulationToggle){
		globals.zModulation = zModulationToggle->selected;
	}
	else if( sender == fullscreenToggle){
		ofSetFullscreen(fullscreenToggle->selected);
		// windows becomes black without this, not sure why...
		//TODO: check if this is still a problem in of0.9
		#ifdef _WIN32
		if(fullscreenToggle->selected){
			int w = ofGetScreenWidth(); 
			int h = ofGetScreenHeight(); 
			ofSetWindowShape(w,h); 
		}
		#endif
	}
	else if( sender == loadFileButton ){
		ofFileDialogResult res = ofSystemLoadDialog("Load audio file", false );
		if( res.bSuccess ){
			ofSendMessage("load:" + res.filePath); 
		}
	}
	else if( sender == useMicButton ){
		if( globals.micActive ){
			ofSendMessage("stop-mic");
		}
		else{
			auto micMenu = new FMenu<string>(0,0,400,0);
			micMenu->bg = ofColor(0,0,255,255);
			micMenu->onAfterRemove.add([](mui::Container * menu, mui::Container * parent) {
				MUI_ROOT->safeDelete(menu); 
			});
			populateMicMenu(micMenu); 

			ofAddListener(micMenu->onSelectOption, this, &PlayerOverlay::inputSelected);
			micMenu->opaque = true; 
			
			MUI_ROOT->showPopupMenu(micMenu, useMicButton, 0, 0, mui::Left, mui::Bottom); 
		}
	}
	else if( sender == timeLabelButton){
		timeLabelMode = (1+timeLabelMode)%2;
	}
	else if (sender == showPlaylistToggle) {
		ofSendMessage(ofMessage("toggle-playlist"));
	}
	else if(sender == analogModeToggle){
		globals.analogMode = globals.analogMode==0?1:0;
	}
}

void PlayerOverlay::inputSelected(const void * sender, FMenu<string>::Option & opt) {
	auto menu = opt.button->findParentOfType<FMenu<string>>();
	auto config_ref = opt.button->getProperty<ma_device_info>("ma_device_info");
	auto type_ref = opt.button->getProperty<ma_device_type>("ma_device_type"); 
	bool withZ = menu->view->findChildrenOfType<mui::ToggleButton>()[0]->selected; 
	if (config_ref && type_ref) {
		selectedMicDeviceInfo.info = *config_ref; 
		selectedMicDeviceInfo.type = *type_ref; 
		//globals.micDeviceId = (*it).second;
		if (withZ) {
			ofSendMessage("start-mic:3");
		}
		else {
			ofSendMessage("start-mic:2");
		}
	}

	MUI_ROOT->safeRemove(menu);
}

PlayerOverlay::mic_info PlayerOverlay::getSelectedMicDeviceInfo() {
  return selectedMicDeviceInfo;
}

void PlayerOverlay::sliderChanged( const void * sender, float & value ){
	if( sender == outputVolumeSlider->slider ){
		globals.outputVolume = outputVolumeSlider->slider->value;
	}
	else if( sender == scaleSlider->slider ){
		globals.scale = scaleSlider->slider->value;
	}
	else if( sender == strokeWeightSlider->slider ){
		globals.strokeWeight = strokeWeightSlider->slider->value;
	}
	else if( sender == timeStretchSlider->slider ){
		globals.timeStretch = timeStretchSlider->slider->value;
	}
	else if( sender == blurSlider->slider ){
		globals.blur = blurSlider->slider->value;
	}
	else if( sender == numPtsSlider->slider ){
		globals.numPts = numPtsSlider->slider->value;
	}
	else if( sender == intensitySlider->slider ){
		globals.intensity = intensitySlider->slider->value;
	}
	else if( sender == afterglowSlider->slider ){
		globals.afterglow = afterglowSlider->slider->value;
	}
}


mui::Label * PlayerOverlay::addLabel( string text ){
	mui::Label * label = new mui::Label( text, 0, 0, 120, 30 );
	label->commit();
	label->width = ceil(label->boundingBox.width);
	label->horizontalAlign = mui::Right;
	add( label );
	return label;
}


string timestring( double secs ){
	int64_t t = secs*1000;
	int millis = t%1000;
	t/=1000;
	
	int seconds = t%60;
	t/=60;
	
	int minutes = t%60;
	t/=60;
	
	stringstream str;
	str << setfill('0') << setw(2) << minutes <<
	":" << setfill('0') << setw(2) << seconds <<
	":" << setfill('0') << setw(2) << (millis/10);
	return str.str();
}

bool filter_mic_menu(const void * sender, bool & value) {
	const mui::ToggleButton * me = (mui::ToggleButton*)sender; 
	return true; 
}

void PlayerOverlay::populateMicMenu(FMenu<string> * menu) {

	mui::ToggleButton * withZ = new mui::ToggleButton("3-channel mode (z-modulation)");
	withZ->label->horizontalAlign = mui::Left; 
	ofAddListener(withZ->onChange, filter_mic_menu);
	withZ->checkbox = true; 
	menu->view->add(withZ); 

	ma_result result;
	ma_context context;
	ma_device_info* pPlaybackDeviceInfos;
	ma_uint32 playbackDeviceCount;
	ma_device_info* pCaptureDeviceInfos;
	ma_uint32 captureDeviceCount;
	ma_uint32 iDevice;

	if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
		printf("Failed to initialize context.\n");
		return;
	}

	result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
	if (result != MA_SUCCESS) {
		printf("Failed to retrieve device information.\n");
		return;
	}


	auto addDevice = [&](string label, string name, ma_device_info info, ma_device_type type) {
		auto btn = menu->addOption(label, name)->button;
		btn->setProperty<ma_device_info>(string("ma_device_info"), move(info));
		btn->setProperty<ma_device_type>(string("ma_device_type"), move(type));
		btn->label->fontSize--;
		printf("    %s\n", info.name);
	}; 

	printf("Capture Devices\n");
	addDevice("Default Microphone", "", ma_device_info(), ma_device_type_capture);

	for (iDevice = 0; iDevice < captureDeviceCount; ++iDevice) {
		ma_device_info dev = pCaptureDeviceInfos[iDevice];
		ma_context_get_device_info(&context, ma_device_type_capture, &dev.id, ma_share_mode_shared, &dev);
		int ch = max(1, (int)max(dev.minChannels, dev.maxChannels));
		addDevice("[" + ofToString(ch) + " ch] " + string(dev.name), dev.name, dev, ma_device_type_capture);
	}


	if (ma_context_is_loopback_supported(&context)) {
		printf("Playback Devices\n");
		// not working: 
		//addDevice("Default Output (looped back)", "", ma_device_info(), ma_device_type_loopback);

		for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
			ma_device_info dev = pPlaybackDeviceInfos[iDevice];
			ma_context_get_device_info(&context, ma_device_type_capture, &dev.id, ma_share_mode_shared, &dev);
			// for now only allow loopbacks in wasapi
			if (context.backend != ma_backend_wasapi) continue;

			int ch = max(1, (int)max(dev.minChannels, dev.maxChannels));
			addDevice("[" + ofToString(ch) + " ch loopback] " + string(dev.name), dev.name, dev, ma_device_type_loopback);
		}
	}

	ma_context_uninit(&context);
}
