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
	mui::Label * emulationLabel;
	mui::SegmentedSelect<int> * emulationMode;

	mui::Label* laserConnectLabel; 
	mui::Button* laserConnect; 
	mui::Label* laserSizeLabel;
	mui::SliderWithLabel* laserSize;
	mui::Label* laserIntensityLabel;
	mui::SliderWithLabel* laserIntensity;
	mui::Label* laserOffsetLabel;
	mui::SliderWithLabel* laserOffsetX;
	mui::SliderWithLabel* laserOffsetY;
	mui::Label* laserKeystoneLabel;
	mui::SliderWithLabel* laserKeystoneX;
	mui::SliderWithLabel* laserKeystoneY;

	void fromGlobals();
	void toGlobals();
	
	void buttonPressed( const void * sender, ofTouchEventArgs & args );
	void emulationModeChanged(const void * sender, int & value);
	
private:
	void pushLabel( string text );
	void outDevicePickerChanged(const void * sender, string & value);
	void gotMessage(const void * sender, ofMessage & msg);
	void addDeviceOptions();

	bool showLaserConfig{ false };
	vector<mui::Container*> hiddenLaserUi;
};
