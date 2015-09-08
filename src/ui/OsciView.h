#pragma once

#include "MUI.h"
#include "../globals.h"
#include "FaButton.h"
#include "FaToggleButton.h"

class OsciView : public mui::Container{
public:
	OsciView( float x_ = 0, float y_ = 0, float width_ = ofGetWidth()/mui::MuiConfig::scaleFactor, float height_ = ofGetHeight()/mui::MuiConfig::scaleFactor );
	
	virtual void update();
	virtual void draw();
	virtual void layout(); 
	
	virtual void touchDown( ofTouchEventArgs &touch );
	virtual void touchMoved( ofTouchEventArgs &touch );
	virtual void touchUp( ofTouchEventArgs &touch );
	virtual void touchDoubleTap( ofTouchEventArgs &touch );
	
	
	FaButton * loadFileButton; 
	
	FaButton * stopButton;
	mui::Slider * scaleSlider;
	
	FaToggleButton * fullscreenButton; 
	FaToggleButton * flipXY;
	FaToggleButton * invertX;
	FaToggleButton * invertY;
	
	FaToggleButton * playButton;
	mui::Slider * timeSlider;
	mui::Slider * volumeSlider;
	
	mui::Label currentTime;
	mui::Label * scaleLabel;
	
	mui::Label * outputVolumeLabel;
	mui::Slider * outputVolumeSlider;
	
	mui::Slider * strokeWeightSlider;
	mui::Label * strokeWeightLabel;
	
	mui::Slider * blurSlider;
	mui::Label * blurLabel;
	
	mui::Slider * numPtsSlider;
	mui::Label * numPtsLabel;
	
	mui::Slider * hueSlider;
	mui::Label * hueLabel;
	
	mui::Slider * intensitySlider;
	mui::Label * intensityLabel;
	
	mui::Slider * afterglowSlider;
	mui::Label * afterglowLabel;
	
	void buttonPressed( const void * sender, ofTouchEventArgs & args );
	
private:
	mui::Label * addLabel( string text );
};
