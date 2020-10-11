#include "ConfigView.h"
#include "../globals.h"
#include "MuiL.h"
#include "miniaudio.h"
#include "../util/WickedLasers.h"

ConfigView::ConfigView()
: mui::Container(0,0, 500, 220){
	bg = ofColor(125, 50);
	opaque = true;

	auto make_slider = [](float vmin, float vmax, float val, int dp) {
		mui::SliderWithLabel* slider = new mui::SliderWithLabel(0, 0, 200, 30, vmin, vmax, val, dp);
		slider->label->fg = ofColor(255);
		return slider;
	};

	auto make_label = [](string text, mui::HorizontalAlign al = mui::Left) {
		mui::Label* label = new mui::Label(text, 0, 0, 200, 30);
		(al==mui::Left?label->inset.left:label->inset.right) = 5;
		label->horizontalAlign = al;
		return label;
	};

	ofAddListener(ofEvents().messageEvent, this, &ConfigView::gotMessage);
	
//	pushLabel( "Audio Device");
	outDevicePicker = new FDropdown<string>(0, 0, 300, 30); 
	outDevicePicker->dataProvider = [&](FDropdown<string> * menu) {
		addDeviceOptions(); 
	}; 
	outDevicePicker->dataDisplay = [&](string name, string value) {
		if (value == "") return string("Default Output"); 
		else return value; 
	}; 
	ofAddListener(outDevicePicker->menu->onSelectValue, this, &ConfigView::outDevicePickerChanged);
	add(outDevicePicker);

	emulationLabel = make_label("Visual Upsampling");
	add(emulationLabel);
	
	emulationMode = new mui::SegmentedSelect<int>(0,0,200,30);
	auto lowChoice = emulationMode->addSegment("Off", 0);
	lowChoice->setProperty("tooltip", string("Disables upsampling. "));
	auto highChoice = emulationMode->addSegment("On", 1);
	highChoice->setProperty("tooltip", string("Enables upsampling. This resembles an analog oscilloscope more closely. "));
	emulationMode->equallySizedButtons = true;
	ofAddListener(emulationMode->onChangeValue, this, &ConfigView::emulationModeChanged);
	add(emulationMode);


	laserConnectLabel = make_label("Wickedlasers Ilda-Adapter");
	add(laserConnectLabel);
	laserConnect = new mui::Button("Show config",0,0,100,30);
	laserConnect->setProperty("tooltip", string("Connect to a wickedlasers ilda-dongle or laserscanner"));
	ofAddListener(laserConnect->onPress, this, &ConfigView::buttonPressed); 
	add(laserConnect);

	laserSizeLabel= make_label("Scale", mui::Right);
	add(laserSizeLabel);
	laserSize = make_slider(0, 1, 0.5, 2);
	laserSize->setProperty("tooltip", string("Size of laser output"));
	add(laserSize);

	laserIntensityLabel= make_label("Intensity", mui::Right);
	add(laserIntensityLabel);
	laserIntensity = make_slider(0, 1, 0, 2);
	add(laserIntensity);

	laserOffsetLabel = make_label("Offset", mui::Right);
	add(laserOffsetLabel);
	laserOffsetX = make_slider(-1, 1, 0, 2);
	add(laserOffsetX);
	laserOffsetY = make_slider(-1, 1, 0, 2);
	add(laserOffsetY);

	laserKeystoneLabel = make_label("Keystone", mui::Right);
	add(laserKeystoneLabel);
	laserKeystoneX = make_slider(-1, 1, 0, 2);
	add(laserKeystoneX);
	laserKeystoneY = make_slider(-1, 1, 0, 2);
	add(laserKeystoneY);

	hiddenLaserUi = {
		laserSizeLabel, laserSize,
		laserIntensityLabel, laserIntensity,
		laserOffsetLabel, laserOffsetX, laserOffsetY,
		laserKeystoneLabel, laserKeystoneX, laserKeystoneY
	};
	for (auto c : hiddenLaserUi) c->visible = false;
}


//--------------------------------------------------------------
void ConfigView::update(){
	auto check = [](float& val, mui::SliderWithLabel* s) {
		if (s->isMouseOver()) val = s->slider->value;
		else s->slider->value = val;
	};
	check(globals.laserSize, laserSize); 
	check(globals.laserIntensity, laserIntensity);
	check(globals.laserKeystoneX, laserKeystoneX);
	check(globals.laserKeystoneY, laserKeystoneY);
	check(globals.laserOffsetX, laserOffsetX);
	check(globals.laserOffsetY, laserOffsetY);
}


//--------------------------------------------------------------
void ConfigView::draw(){
}

//--------------------------------------------------------------
void ConfigView::layout(){
	float lw = 180;

	mui::L(outDevicePicker).posTL(5,5).stretchToRightEdgeOfParent(5);
	mui::L(emulationLabel).below(outDevicePicker,5);
	mui::L(emulationMode).rightOf(emulationLabel).alignRightEdgeTo(outDevicePicker);

	mui::L(laserConnectLabel).below(emulationLabel, 15).width(300);
	mui::L(laserConnect).rightOf(laserConnectLabel).alignRightEdgeTo(emulationMode); 

	mui::L(laserSizeLabel).below(laserConnectLabel, 5).width(lw);
	mui::L(laserSize).rightOf(laserSizeLabel).stretchToRightEdgeOfParent(5); 

	mui::L(laserIntensityLabel).below(laserSizeLabel).width(lw);
	mui::L(laserIntensity).rightOf(laserIntensityLabel).stretchToRightEdgeOfParent(5);

	mui::L(laserOffsetLabel).below(laserIntensityLabel).width(lw);
	mui::L(laserOffsetX).rightOf(laserOffsetLabel).stretchToRightEdgeOfParent(5);
	mui::L(laserOffsetY).below(laserOffsetX).stretchToRightEdgeOfParent(5);

	mui::L(laserKeystoneLabel).below(laserOffsetY).alignLeftEdgeTo(laserOffsetLabel).width(lw);
	mui::L(laserKeystoneX).rightOf(laserKeystoneLabel).stretchToRightEdgeOfParent(5);
	mui::L(laserKeystoneY).below(laserKeystoneX).stretchToRightEdgeOfParent(5);

	height = getChildBounds().getBottom() + 10;

}


//--------------------------------------------------------------
void ConfigView::touchDown( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void ConfigView::touchMoved( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void ConfigView::touchUp( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void ConfigView::touchDoubleTap( ofTouchEventArgs &touch ){
}

//--------------------------------------------------------------
void ConfigView::fromGlobals(){
	outDevicePicker->setSelectedValue( globals.out_requested.name );
	emulationMode->setSelected(globals.analogMode==0?0:1);
}

//--------------------------------------------------------------
void ConfigView::toGlobals(){
	globals.out_requested.name = outDevicePicker->getSelectedValueOr("");
	globals.analogMode = emulationMode->getSelectedValueOr(1);
}


//--------------------------------------------------------------
void ConfigView::pushLabel( string text ){
	mui::Label * label = new mui::Label( text, 0, 0, 100, 12 );
	label->fontSize = 8; 
	add( label );
}


//--------------------------------------------------------------
void ConfigView::buttonPressed( const void * sender, ofTouchEventArgs & args ){
	if (sender == laserConnect) {
		if (!showLaserConfig) {
			for (auto c : hiddenLaserUi) c->visible = true;
			laserConnect->label->setText("Connect");
			MUI_ROOT->handleLayout();
			showLaserConfig = true; 
		}
		else{
			if (globals.laserConnected) {
				globals.laserPtr->disconnect();
			}
			else {
				globals.laserPtr->connect();
			}

			laserConnect->label->setText(globals.laserConnected ? "Disconnect" : "Connect");
		}
	}
}

//--------------------------------------------------------------
void ConfigView::outDevicePickerChanged(const void * sender, string & value) {
	ofSendMessage("out-choice-changed"); 
}

//--------------------------------------------------------------
void ConfigView::emulationModeChanged(const void * sender, int & value){
	globals.analogMode = value;
}

//--------------------------------------------------------------
void ConfigView::gotMessage(const void * sender, ofMessage & msg) {
}


//--------------------------------------------------------------
void ConfigView::addDeviceOptions() {
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
		return;
	}

	outDevicePicker->addOption("Default Output", ""); 

	printf("Playback Devices\n");
	for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
		ma_device_info dev = pPlaybackDeviceInfos[iDevice];
		ma_context_get_device_info(&context, ma_device_type_playback, &dev.id, ma_share_mode_shared, &dev);
		auto btn = outDevicePicker->addOption(dev.name, dev.name)->button;
		btn->setProperty<ma_device_info>(string("ma_device_info"), move(dev));
		printf("    %u: %s\n", iDevice, pPlaybackDeviceInfos[iDevice].name);
	}
}
