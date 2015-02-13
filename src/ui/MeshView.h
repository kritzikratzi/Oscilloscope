#pragma once

#include "MUI.h"
#include "settings.h"


class MeshView : public mui::Container{
public:
	MeshView( float x_ = 0, float y_ = 0, float width_ = ofGetWidth(), float height_ = ofGetHeight() );
	
	virtual void update();
	virtual void draw();
	virtual void drawBackground();
	
	virtual void touchDown( ofTouchEventArgs &touch );
	virtual void touchMoved( ofTouchEventArgs &touch );
	virtual void touchUp( ofTouchEventArgs &touch );
	virtual void touchDoubleTap( ofTouchEventArgs &touch );
	
	mui::Button * stopButton;
	mui::SliderWithLabel * scaleSlider;
	
	mui::ToggleButton * flipXY;
	mui::ToggleButton * invertX;
	mui::ToggleButton * invertY;

	mui::SliderWithLabel * lineWidth;
	mui::SliderWithLabel * clearBg;

	void buttonPressed( const void * sender, ofTouchEventArgs & args );
	
private:
	
	void pushLabel( string text, float &x, float &y, float &w, float &h );
};
