#pragma once

#include "ofxMightyUI.h"
#include "FMenu.h"

class ConfigView : public mui::Container{
public:
	ConfigView();
	
	void update() override;
	void draw() override;
	void layout() override; 

	void touchDown( ofTouchEventArgs &touch ) override;
	void touchMoved( ofTouchEventArgs &touch ) override;
	void touchUp( ofTouchEventArgs &touch ) override;
	void touchDoubleTap( ofTouchEventArgs &touch ) override;
	
	
	FDropdown<string> * outDevicePicker; 
	FDropdown<int> * sampleRatePicker;

	mui::Button * closeButton;
	
	void fromGlobals();
	void toGlobals();
	
	void buttonPressed( const void * sender, ofTouchEventArgs & args );
	
private:
	void pushLabel( string text );
	void outDevicePickerChanged(const void * sender, string & value);
	void sampleRatePickerChanged(const void * sender, int & value);
	void gotMessage(const void * sender, ofMessage & msg);
	void addDeviceOptions();
};
