#include "ConfigView.h"
#include <Poco/Delegate.h>
#include "../util/sounddevices.h"
#include "../globals.h"

ConfigView::ConfigView( float x_, float y_, float width_, float height_)
: mui::Container( x_, y_, width_, height_ ){
	float x = 10, y = 10, w = 400, h = 30, space = 10;
	
	pushLabel( "Auto Detect", x, y, w, h );
	autoDetectButton = new mui::ToggleButton( "Use system default", x, y, w, h );
	add( autoDetectButton );
	ofAddListener( autoDetectButton->onPress, this, &ConfigView::buttonPressed );
	y += autoDetectButton->height + space;
	
	pushLabel( "Sample Rate", x, y, w, h);
	sampleRatesSelect = new mui::SegmentedSelect<int>( x, y, w, h );
	sampleRatesSelect->addSegment("44100", 44100);
	sampleRatesSelect->addSegment("48000", 48000);
	sampleRatesSelect->addSegment("88200", 88200);
	sampleRatesSelect->addSegment("96000", 96000);
	sampleRatesSelect->addSegment("176400", 176400);
	sampleRatesSelect->addSegment("192000", 192000);
	sampleRatesSelect->commit();
	add( sampleRatesSelect );
	y += sampleRatesSelect->height + space;
	
	pushLabel( "Buffer Size", x, y, w, h);
	bufferSizeSelect = new mui::SegmentedSelect<int>( x, y, w, h );
	bufferSizeSelect->addSegment("256", 256);
	bufferSizeSelect->addSegment("512", 512);
	bufferSizeSelect->addSegment("1024", 1024);
	bufferSizeSelect->addSegment("2048", 2048);
	bufferSizeSelect->addSegment("4096", 4096);
	bufferSizeSelect->commit();
	add( bufferSizeSelect );
	y += bufferSizeSelect->height + space;
	
	pushLabel( "Number of Buffers", x, y, w, h);
	numbuffersSelect = new mui::SegmentedSelect<int>( x, y, w, h );
	numbuffersSelect->addSegment("1", 1);
	numbuffersSelect->addSegment("2", 2);
	numbuffersSelect->addSegment("3", 3);
	numbuffersSelect->addSegment("4", 4);
	numbuffersSelect->addSegment("5", 5);
	numbuffersSelect->addSegment("6", 6);
	numbuffersSelect->commit(); 
	add( numbuffersSelect );
	y += numbuffersSelect->height + space;
	
	pushLabel( "Audio Device", x, y, w, h);
	vector<ofSoundDevice> infos = ofSoundStream().getDeviceList();
	vector<ofSoundDevice>::iterator it = infos.begin();
	while( it != infos.end() ){
		ofSoundDevice info = *it;
		string name = string(info.name);
		if( info.outputChannels > 0 ){
			mui::ToggleButton * button = new mui::ToggleButton( name, x, y, w, h );
			deviceIds[button] = static_cast<int>(it-infos.begin());
			ofAddListener( button->onPress, this, &ConfigView::buttonPressed );
			button->fg = ofColor( 255 );
			button->label->horizontalAlign = mui::Left;
			soundcardButtons.push_back(button);
			add( button );
			y += button->height;
		}
		
		
		++it;
	}
	
	y += 20;
	
	float startY  = autoDetectButton->y + autoDetectButton->height;
	blocker = new mui::Container( x, startY, w, y - startY - 20 );
	blocker->bg = ofColor(0, 150);
	blocker->ignoreEvents = false;
	blocker->opaque = true;
	add( blocker );
	
	startButton = new mui::Button( "Start!", x, y, w, h );
	ofAddListener( startButton->onPress, this, &ConfigView::buttonPressed );

	y += startButton->height;
	add( startButton );
	startButton->requestKeyboardFocus();
}


//--------------------------------------------------------------
void ConfigView::update(){
	blocker->visible = autoDetectButton->selected;
}


//--------------------------------------------------------------
void ConfigView::draw(){
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
	autoDetectButton->selected = globals.autoDetect;
	sampleRatesSelect->setSelected( globals.sampleRate );
	bufferSizeSelect->setSelected( globals.bufferSize );
	numbuffersSelect->setSelected(globals.numBuffers );

	if( (size_t)globals.deviceId < soundcardButtons.size() ){
		selectSoundCard( globals.deviceId );
	}
	else{
		selectSoundCard( -1 );
	}
	
	sampleRatesSelect->commit();
	bufferSizeSelect->commit();
	numbuffersSelect->commit();
}

//--------------------------------------------------------------
void ConfigView::toGlobals(){
	globals.autoDetect = autoDetectButton->selected;
	globals.sampleRate = sampleRatesSelect->getSelectedValueOr(44100);
	globals.bufferSize = bufferSizeSelect->getSelectedValueOr(512);
	globals.numBuffers = numbuffersSelect->getSelectedValueOr(4);
	globals.deviceId = selectedSoundCard;
}


//--------------------------------------------------------------
void ConfigView::pushLabel( string text, float &x, float &y, float &w, float &h ){
	mui::Label * label = new mui::Label( text, x, y, w, h );
	add( label );
	y += label->height;
}


//--------------------------------------------------------------
void ConfigView::buttonPressed( const void * sender, ofTouchEventArgs & args ){
	if( sender == autoDetectButton ){
		if( autoDetectButton->selected){
			autoDetect();
		}
	}
	else if( sender == startButton ){
		ofBaseApp * app = ofGetAppPtr();
		app->gotMessage( ofMessage( "start-pressed" ) );
	}
	else{
		mui::ToggleButton * btn = (mui::ToggleButton*)sender;
		selectSoundCard(deviceIds[btn]);
	}
}

//--------------------------------------------------------------
bool ConfigView::keyPressed( ofKeyEventArgs &key){
	if(key.key == OF_KEY_RETURN){
		startButton->clickAndNotify();
		return true; 
	}
	else return false; 
}


void ConfigView::selectSoundCard( int deviceId ){
	if( deviceId < 0 && !soundcardButtons.empty() ){
		deviceId = deviceIds[soundcardButtons.front()];
	}
	
	for( vector<mui::ToggleButton*>::iterator it = soundcardButtons.begin(); it != soundcardButtons.end(); ++it ){
		mui::ToggleButton * btn = *it;
		if( deviceIds[btn] == deviceId ){
			selectedSoundCard = deviceId;
			btn->selected = true;
		}
		else{
			btn->selected = false;
		}
	}
}


void ConfigView::autoDetect(){
	int deviceId = 0;
	int sampleRate = 44100;
	int bufferSize = 512;
	int numBuffers = 4;
	getDefaultRtOutputParams(deviceId, sampleRate, bufferSize, numBuffers);
	if( (size_t)deviceId < soundcardButtons.size() ) selectSoundCard(deviceId);
	sampleRatesSelect->setSelected(sampleRate);
	bufferSizeSelect->setSelected(bufferSize);
	numbuffersSelect->setSelected(numBuffers);
}