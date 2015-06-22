#include "OsciView.h"
#include <Poco/Delegate.h>
#include "sounddevices.h"

OsciView::OsciView( float x_, float y_, float width_, float height_)
: mui::Container( x_, y_, width_, height_ ){
	float x = 10, y = 10, w = 400, h = 30;
	
	stopButton = new mui::Button( "Stop!", x, y, w, h );
	ofAddListener( stopButton->onPress, this, &OsciView::buttonPressed );
	y += stopButton->height + 10;
	add( stopButton );
	
	pushLabel("Scale", x, y, w, h );
	scaleSlider = new mui::SliderWithLabel( x, y, w, h, 0.1, 10, 1, 2 );
	scaleSlider->label->fg = ofColor( 255 );
	y += scaleSlider->height + 10;
	add( scaleSlider );
	
	
	pushLabel( "Channels", x, y, w, h );
	w = 90;
	flipXY = new mui::ToggleButton( "Flip X/Y", x, y, w, h );
	add( flipXY );
	x += 100;
	
	invertX = new mui::ToggleButton( "Invert X", x, y, w, h );
	add( invertX );
	x += 100;
	
	invertY = new mui::ToggleButton( "Invert Y", x, y, w, h );
	add( invertY );
	x += 100;
	
	x = 10;
	y += invertY->height + 10;
	
}


//--------------------------------------------------------------
void OsciView::update(){
}


//--------------------------------------------------------------
void OsciView::draw(){
}


//--------------------------------------------------------------
void OsciView::drawBackground(){
}


//--------------------------------------------------------------
void OsciView::touchDown( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void OsciView::touchMoved( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void OsciView::touchUp( ofTouchEventArgs &touch ){
}


//--------------------------------------------------------------
void OsciView::touchDoubleTap( ofTouchEventArgs &touch ){
}

//--------------------------------------------------------------
void OsciView::pushLabel( string text, float &x, float &y, float &w, float &h ){
	mui::Label * label = new mui::Label( text, x, y, w, h );
	add( label );
	y += label->height;
}


//--------------------------------------------------------------
void OsciView::buttonPressed( const void * sender, ofTouchEventArgs & args ){
	if( sender == stopButton ){
		ofBaseApp * app = ofGetAppPtr();
		app->gotMessage( ofMessage( "stop-pressed" ) );
	}
}
