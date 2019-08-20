#include "ConfigView.h"
#include "../globals.h"
#include "MuiL.h"
#include "miniaudio.h"

ConfigView::ConfigView()
: mui::Container(0,0, 500, 40){
	bg = ofColor(125, 50);
	opaque = true;

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
}


//--------------------------------------------------------------
void ConfigView::update(){
}


//--------------------------------------------------------------
void ConfigView::draw(){
}

//--------------------------------------------------------------
void ConfigView::layout(){
	mui::L(outDevicePicker).posTL(5,5).stretchToRightEdgeOfParent(5);
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
}

//--------------------------------------------------------------
void ConfigView::toGlobals(){
	globals.out_requested.name = outDevicePicker->getSelectedValueOr("");
}


//--------------------------------------------------------------
void ConfigView::pushLabel( string text ){
	mui::Label * label = new mui::Label( text, 0, 0, 100, 12 );
	label->fontSize = 8; 
	add( label );
}


//--------------------------------------------------------------
void ConfigView::buttonPressed( const void * sender, ofTouchEventArgs & args ){
}

//--------------------------------------------------------------
void ConfigView::outDevicePickerChanged(const void * sender, string & value) {
	ofSendMessage("out-choice-changed"); 
}

void ConfigView::gotMessage(const void * sender, ofMessage & msg) {
}


//--------------------------------------------------------------
void ConfigView::addDeviceOptions() {
	ma_context context;
	ma_device_info* pPlaybackDeviceInfos;
	ma_uint32 playbackDeviceCount;
	ma_device_info* pCaptureDeviceInfos;
	ma_uint32 captureDeviceCount;
	ma_uint32 iDevice;
	ma_result result; 

	if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
		printf("Failed to initialize context.\n");
		return;
	}
	
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

	ma_context_uninit(&context);
}
