#include "ConfigView.h"
#include <Poco/Delegate.h>
#include "sounddevices.h"
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
	sampleRatesSelect = new mui::SegmentedSelect( x, y, w, h );
	sampleRatesSelect->addLabel("44100");
	sampleRatesSelect->addLabel("48000");
	sampleRatesSelect->addLabel("88200");
	sampleRatesSelect->addLabel("96000");
	sampleRatesSelect->addLabel("176400");
	sampleRatesSelect->addLabel("192000");
	sampleRatesSelect->commit();
	add( sampleRatesSelect );
	y += sampleRatesSelect->height + space;
	
	pushLabel( "Buffer Size", x, y, w, h);
	bufferSizeSelect = new mui::SegmentedSelect( x, y, w, h );
	bufferSizeSelect->addLabel("256");
	bufferSizeSelect->addLabel("512");
	bufferSizeSelect->addLabel("1024");
	bufferSizeSelect->addLabel("2048");
	bufferSizeSelect->addLabel("4096");
	bufferSizeSelect->commit();
	add( bufferSizeSelect );
	y += bufferSizeSelect->height + space;
	
	pushLabel( "Number of Buffers", x, y, w, h);
	numbuffersSelect = new mui::SegmentedSelect( x, y, w, h );
	numbuffersSelect->addLabel("1");
	numbuffersSelect->addLabel("2");
	numbuffersSelect->addLabel("3");
	numbuffersSelect->addLabel("4");
	numbuffersSelect->addLabel("5");
	numbuffersSelect->addLabel("6");
	numbuffersSelect->commit(); 
	add( numbuffersSelect );
	y += numbuffersSelect->height + space;
	
	pushLabel( "Audio Device", x, y, w, h);
	vector<RtAudio::DeviceInfo> infos = listRtSoundDevices();
	vector<RtAudio::DeviceInfo>::iterator it = infos.begin();
	while( it != infos.end() ){
		RtAudio::DeviceInfo info = *it;
		string name = string(info.name) + ": out=" + ofToString(info.outputChannels) + ",in=" + ofToString(info.inputChannels); 
		mui::ToggleButton * button = new mui::ToggleButton( name, x, y, w, h );
		ofAddListener( button->onPress, this, &ConfigView::buttonPressed );
		button->fg = ofColor( 255 );
		button->label->horizontalAlign = mui::Left;
		soundcardButtons.push_back(button);
		add( button );
		y += button->height;
		
		++it;
	}
	
	y += 20;

	
	startButton = new mui::Button( "Start!", x, y, w, h );
	ofAddListener( startButton->onPress, this, &ConfigView::buttonPressed );
	y += startButton->height; 
	add( startButton );
}


//--------------------------------------------------------------
void ConfigView::update(){
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
	sampleRatesSelect->selected = ofToString( globals.sampleRate );
	bufferSizeSelect->selected = ofToString( globals.bufferSize );
	numbuffersSelect->selected = ofToString( globals.numBuffers );

	if( globals.deviceId < soundcardButtons.size() ){
		selectSoundCard( soundcardButtons[globals.deviceId] );
	}
	else{
		selectSoundCard( soundcardButtons[0] );
	}
	
	sampleRatesSelect->commit();
	bufferSizeSelect->commit();
	numbuffersSelect->commit();
}

//--------------------------------------------------------------
void ConfigView::toGlobals(){
	globals.autoDetect = autoDetectButton->selected;
	globals.sampleRate = ofToInt( sampleRatesSelect->selected );
	globals.bufferSize = ofToInt( bufferSizeSelect->selected );
	globals.numBuffers = ofToInt( numbuffersSelect->selected );
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
		selectSoundCard((mui::ToggleButton*)sender);
	}
}

void ConfigView::selectSoundCard( mui::ToggleButton * card ){
	for( int i = 0; i < soundcardButtons.size(); i++ ){
		mui::ToggleButton * btn = soundcardButtons[i];
		if( btn == card ){
			selectedSoundCard = i;
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
	if( deviceId < soundcardButtons.size() ) selectSoundCard(soundcardButtons[deviceId]);
	sampleRatesSelect->selected = ofToString(sampleRate);
	sampleRatesSelect->commit();
	bufferSizeSelect->selected = ofToString(bufferSize);
	bufferSizeSelect->commit();
	numbuffersSelect->selected = ofToString(numBuffers);
	numbuffersSelect->commit();
}