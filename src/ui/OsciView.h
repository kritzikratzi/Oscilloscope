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
	
	void buttonPressed( const void * sender, ofTouchEventArgs & args );
	
private:
	
	void pushLabel( string text, float &x, float &y, float &w, float &h );
};
