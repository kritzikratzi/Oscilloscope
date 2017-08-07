#pragma once

#include "ofxMightyUI.h"
#include "../globals.h"
#include "FaButton.h"
#include "FaToggleButton.h"
#include "FMenu.h"


class PlayerOverlay : public mui::Container{
public:
	PlayerOverlay( float x_ = 0, float y_ = 0, float width_ = ofGetWidth()/mui::MuiConfig::scaleFactor, float height_ = ofGetHeight()/mui::MuiConfig::scaleFactor );
	
	virtual void fromGlobals(); 
	
	virtual void update();
	virtual void draw();
	virtual void layout(); 
	
	virtual void touchDown( ofTouchEventArgs &touch );
	virtual void touchMoved( ofTouchEventArgs &touch );
	virtual void touchUp( ofTouchEventArgs &touch );
	virtual void touchDoubleTap( ofTouchEventArgs &touch );
	
	FMenu * micMenu; 
	map<string,int> micDeviceIds;
	
	FaButton * loadFileButton; 
	FaToggleButton * useMicButton;
	
	FaButton * stopButton;
	mui::SliderWithLabel * scaleSlider;
	
	FaToggleButton * fullscreenButton; 
	FaToggleButton * flipXY;
	FaToggleButton * invertX;
	FaToggleButton * invertY;
	FaToggleButton * sideBySide;
	FaToggleButton * flip3d; 

	FaToggleButton * playButton;
	mui::Slider * timeSlider;
	mui::Button * timeLabelButton;
	int timeLabelMode = 0; 
	
	mui::Label currentTime;
	mui::Label * scaleLabel;
	
	mui::Label * outputVolumeLabel;
	mui::SliderWithLabel * outputVolumeSlider;
	
	mui::SliderWithLabel * strokeWeightSlider;
	mui::Label * strokeWeightLabel;
	
	mui::SliderWithLabel * timeStretchSlider;
	mui::Label * timeStretchLabel;
	
	mui::SliderWithLabel * blurSlider;
	mui::Label * blurLabel;
	
	mui::SliderWithLabel * numPtsSlider;
	mui::Label * numPtsLabel;
	
	mui::SliderWithLabel * hueSlider;
	mui::Label * hueLabel;
	
	mui::SliderWithLabel * intensitySlider;
	mui::Label * intensityLabel;
	
	mui::SliderWithLabel * afterglowSlider;
	mui::Label * afterglowLabel;
	
	void buttonPressed( const void * sender, ofTouchEventArgs & args );
	void sliderChanged( const void * sender, float & value );
	
private:
	mui::Label * addLabel( string text );
};
