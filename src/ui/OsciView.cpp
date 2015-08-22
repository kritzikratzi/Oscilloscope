#include "OsciView.h"
#include <Poco/Delegate.h>
#include "sounddevices.h"
#include "ofxFontAwesome.h"
#include "ofApp.h"
#include "L.h"

OsciView::OsciView( float x_, float y_, float width_, float height_)
: mui::Container( x_, y_, width_, height_ ){
	float x = 10, y = 10, w = 400, h = 30;
	bg = ofColor(125,50);
	opaque = true;

	string xx = ofxFontAwesome::play; 
	
	stopButton = new FaButton( ofxFontAwesome::cogs, x, y, h, h );
	ofAddListener( stopButton->onPress, this, &OsciView::buttonPressed );
	y += stopButton->height + 10;
	add( stopButton );
	
	scaleLabel = new mui::Label( "Scale", 0, 0, 100, h );
	scaleLabel->fontSize = 10;
	scaleLabel->commit();
	scaleLabel->width = scaleLabel->boundingBox.width;
	scaleLabel->horizontalAlign = mui::Right;
	add(scaleLabel);
	
	scaleSlider = new mui::Slider( x, y, w, h, 0.1, 2, 1 );
	y += scaleSlider->height + 10;
	add( scaleSlider );
	
	
	w = 90;
	flipXY = new FaToggleButton( ofxFontAwesome::repeat, ofxFontAwesome::repeat, x, y, h, h );
	ofAddListener( flipXY->onPress, this, &OsciView::buttonPressed );
	add( flipXY );
	x += 100;
	
	invertX = new FaToggleButton( ofxFontAwesome::arrows_h, ofxFontAwesome::arrows_h, x, y, h, h );
	ofAddListener( invertX->onPress, this, &OsciView::buttonPressed );
	add( invertX );
	x += 100;
	
	invertY = new FaToggleButton( ofxFontAwesome::arrows_v, ofxFontAwesome::arrows_v, x, y, h, h );
	ofAddListener( invertY->onPress, this, &OsciView::buttonPressed );
	add( invertY );
	x += 100;
	
	fullscreenButton = new FaToggleButton( ofxFontAwesome::expand, ofxFontAwesome::compress, x, y, h, h );
	ofAddListener( fullscreenButton->onPress, this, &OsciView::buttonPressed );
	add( fullscreenButton );
	
	loadFileButton = new FaButton( ofxFontAwesome::folder_open, x, y, h, h );
	ofAddListener( loadFileButton->onPress, this, &OsciView::buttonPressed );
	add( loadFileButton );
	
	playButton = new FaToggleButton( ofxFontAwesome::play, ofxFontAwesome::pause, x, y, h, h );
	ofAddListener( playButton->onPress, this, &OsciView::buttonPressed );
	add( playButton );
	
	x = 10;
	y += invertY->height + 10;
	
	timeSlider = new mui::Slider( 0, 0, w, h );
	add( timeSlider );
	
	
	outputVolumeLabel = new mui::Label( "Volume", 0, 0, 100, h );
	outputVolumeLabel->fontSize = 10;
	outputVolumeLabel->commit();
	outputVolumeLabel->width  = outputVolumeLabel->boundingBox.width;
	outputVolumeLabel->horizontalAlign = mui::Right;

	add( outputVolumeLabel);
	
	outputVolumeSlider = new mui::Slider(0, 0, 100, h, 0, 1, 0.8 );
	add( outputVolumeSlider);
	
	layout();
}

void OsciView::layout(){
	// make labels same width
	float w = max(outputVolumeLabel->width, scaleLabel->width);
	outputVolumeLabel->width = scaleLabel->width = w;
	
	mui::L(fullscreenButton).pos(width-30, 0);
	mui::L(stopButton).leftOf(fullscreenButton,1);
	
	mui::L(playButton).pos(10,40);
	mui::L(timeSlider).rightOf(playButton, 10).stretchToRightEdgeOf(this, 10);
	
	mui::L(loadFileButton).below(playButton, 10);
	mui::L(outputVolumeLabel).rightOf(loadFileButton,20);
	
	mui::L(invertX).below(loadFileButton, 10);
	mui::L(invertY).rightOf(invertX, 10);
	mui::L(flipXY).rightOf(invertY,10);
	mui::L(scaleLabel).rightOf(flipXY,20);
	
	
	outputVolumeLabel->x = scaleLabel->x = max( scaleLabel->x, outputVolumeLabel->x );
	mui::L(outputVolumeSlider).rightOf(outputVolumeLabel,5);
	mui::L(scaleSlider).rightOf(scaleLabel,5);
	outputVolumeSlider->x = scaleSlider->x = max( outputVolumeSlider->x, scaleSlider->x );
	
	mui::L(outputVolumeSlider).stretchToRightEdgeOf(this,10);
	mui::L(scaleSlider).stretchToRightEdgeOf(this,10);
}


// tiny helper functions.
// these should somehow be in mui, they're so handy
// returns false if the user changed the value
bool updateSlider( mui::Slider * slider, float targetValue, float &lastValue ){
	if( slider->hasFocus() ){
		if( lastValue != slider->value ){
			lastValue = slider->value;
			return false;
		}
		else{
			return true;
		}
	}
	else{
		slider->value = targetValue;
		return true;
	}
	
}

//--------------------------------------------------------------
void OsciView::update(){
	static float lastTimeVal = -1;
	if( !updateSlider(timeSlider, globals.player.getPosition(), lastTimeVal ) ){
		globals.player.setPosition(timeSlider->value);
	}

	static float lastOutputVol = -1;
	if( !updateSlider(outputVolumeSlider, globals.outputVolume, lastOutputVol) ){
		globals.outputVolume = outputVolumeSlider->value;
	}
	
	static float lastScaleVal = -1;
	if( !updateSlider(scaleSlider, globals.scale, lastScaleVal) ){
		globals.scale = scaleSlider->value;
	}
	
	if( globals.player.isPlaying != playButton->selected ){
		playButton->selected = globals.player.isPlaying;
		playButton->commit();
	}
	
	if( globals.invertX != invertX->selected ){
		invertX->selected = globals.invertX;
		invertX->commit();
	}
	
	if( globals.invertY != invertY->selected ){
		invertY->selected = globals.invertY;
		invertY->commit();
	}
	
	if( globals.flipXY != flipXY->selected ){
		flipXY->selected = globals.flipXY;
		flipXY->commit();
	}
	
	
	
}


//--------------------------------------------------------------
void OsciView::draw(){
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
	else if( sender == playButton ){
		if( globals.player.isPlaying ){
			globals.player.stop();
		}
		else{
			globals.player.play();
		}
	}
	else if( sender == invertX ){
		globals.invertX = invertX->selected;
	}
	else if( sender == invertY ){
		globals.invertY = invertY->selected;
	}
	else if( sender == flipXY ){
		globals.flipXY = flipXY->selected;
	}
	else if( sender == fullscreenButton ){
		ofSetFullscreen(fullscreenButton->selected);
	}
	else if( sender == loadFileButton ){
		ofFileDialogResult res = ofSystemLoadDialog("Load audio file", false );
		if( res.bSuccess ){
			globals.player.loadSound(res.filePath);
		}
	}
}
