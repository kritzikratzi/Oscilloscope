#pragma once

#include "ofxMightyUI.h"

class ConfigView : public mui::Container{
public:
	ConfigView( float x_ = 0, float y_ = 0, float width_ = ofGetWidth(), float height_ = ofGetHeight() );
	
	virtual void update();
	virtual void draw();
	
	virtual void touchDown( ofTouchEventArgs &touch );
	virtual void touchMoved( ofTouchEventArgs &touch );
	virtual void touchUp( ofTouchEventArgs &touch );
	virtual void touchDoubleTap( ofTouchEventArgs &touch );
	
	
	mui::ToggleButton * autoDetectButton;
	mui::SegmentedSelect<int> * sampleRatesSelect;
	mui::SegmentedSelect<int> * bufferSizeSelect;
	mui::SegmentedSelect<int> * numbuffersSelect;
	mui::Button * startButton;
	
	mui::Container * blocker;
	
	void fromGlobals();
	void toGlobals();
	
	void buttonPressed( const void * sender, ofTouchEventArgs & args ); 
	void selectSoundCard( int deviceId );
	void autoDetect();
	
private:
	
	void pushLabel( string text, float &x, float &y, float &w, float &h );
	int selectedSoundCard;
	vector<mui::ToggleButton*> soundcardButtons;
	map<mui::ToggleButton*, int> deviceIds;
};
