#pragma once

#include "MUI.h"
#include "../globals.h"
#include "FaButton.h"
#include "FaToggleButton.h"

class OsciView : public mui::Container{
public:
	OsciView( float x_ = 0, float y_ = 0, float width_ = ofGetWidth()/mui::MuiConfig::scaleFactor, float height_ = ofGetHeight()/mui::MuiConfig::scaleFactor );
	
	virtual void fromGlobals(); 
	
	virtual void update();
	virtual void draw();
	virtual void layout(); 
	
	virtual void touchDown( ofTouchEventArgs &touch );
	virtual void touchMoved( ofTouchEventArgs &touch );
	virtual void touchUp( ofTouchEventArgs &touch );
	virtual void touchDoubleTap( ofTouchEventArgs &touch );
	
	
	FaButton * loadFileButton; 
	
	FaButton * stopButton;
	mui::SliderWithLabel * scaleSlider;
	
	FaToggleButton * fullscreenButton; 
	FaToggleButton * flipXY;
	FaToggleButton * invertX;
	FaToggleButton * invertY;
	
	FaToggleButton * playButton;
	mui::Slider * timeSlider;
	
	mui::Label currentTime;
	mui::Label * scaleLabel;
	
	mui::Label * outputVolumeLabel;
	mui::SliderWithLabel * outputVolumeSlider;
	
	mui::SliderWithLabel * strokeWeightSlider;
	mui::Label * strokeWeightLabel;
	
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
