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
	
	scaleLabel = addLabel( "Scale" );
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
	
	
	outputVolumeLabel = addLabel( "Volume" );
	outputVolumeSlider = new mui::Slider(0, 0, 100, h, 0, 1, 0.8 );
	add(outputVolumeSlider);
	
	strokeWeightLabel = addLabel( "Stroke Weight" );
	strokeWeightSlider = new mui::Slider(0, 0, 100, h, 1, 20, 10 );
	add( strokeWeightSlider );
	
	blurLabel = addLabel( "Blur" );
	blurSlider = new mui::Slider(0,0, 100, h, 0, 255, 30);
	add(blurSlider);
	
	numPtsLabel = addLabel( "Points" );
	numPtsSlider = new mui::Slider(0,0,100,h, 1, 100, 20);
	add(numPtsSlider);

	hueLabel = addLabel( "Hue" );
	hueSlider = new mui::Slider(0,0,100,h,0,360, 0);
	add(hueSlider);
	
	intensityLabel = addLabel( "Intensity" );
	intensitySlider = new mui::Slider(0,0,100,h,0,1, 0);
	add(intensitySlider);
	
	afterglowLabel = addLabel( "Afterglow" );
	afterglowSlider = new mui::Slider(0,0,100,h,0,1, 0);
	add(afterglowSlider);
	
	
	

	
	layout();
}

void OsciView::layout(){
	mui::L(fullscreenButton).pos(width-30, 0);
	mui::L(stopButton).leftOf(fullscreenButton,1);
	
	mui::L(playButton).pos(10,40);
	mui::L(timeSlider).rightOf(playButton, 10).stretchToRightEdgeOf(this, 10);
	
	mui::L(loadFileButton).below(playButton, 10);
	mui::L(outputVolumeLabel).rightOf(loadFileButton,20);
	mui::L(outputVolumeSlider).rightOf(outputVolumeLabel,5).stretchToRightEdgeOf(this,10);
	
	mui::L(invertX).below(loadFileButton, 10);
	mui::L(invertY).rightOf(invertX, 10);
	mui::L(flipXY).rightOf(invertY,10);
	mui::L(scaleLabel).rightOf(flipXY,20);
	mui::L(scaleSlider).rightOf(scaleLabel,5).stretchToRightEdgeOf(this,10);
	
	mui::L(strokeWeightLabel).below(scaleLabel).alignRightEdgeTo(scaleLabel);
	mui::L(strokeWeightSlider).rightOf(strokeWeightLabel,5).stretchToRightEdgeOf(this,10);
	
	/*mui::L(blurLabel).below(strokeWeightLabel).alignRightEdgeTo(strokeWeightLabel);
	mui::L(blurSlider).rightOf(blurLabel,5).stretchToRightEdgeOf(this,10);
	
	mui::L(numPtsLabel).below(blurLabel).alignRightEdgeTo(blurLabel);
	mui::L(numPtsSlider).rightOf(numPtsLabel,5).stretchToRightEdgeOf(this,10);*/
	blurLabel->visible = false;
	blurSlider->visible = false;
	numPtsLabel->visible = false;
	numPtsSlider->visible = false;
	
	
	mui::L(hueLabel).below(strokeWeightLabel).alignRightEdgeTo(strokeWeightLabel);
	mui::L(hueSlider).rightOf(hueLabel,5).stretchToRightEdgeOf(this,10);
	
	mui::L(intensityLabel).below(hueLabel).alignRightEdgeTo(hueLabel);
	mui::L(intensitySlider).rightOf(intensityLabel,5).stretchToRightEdgeOf(this,10);
	
	mui::L(afterglowLabel).below(intensityLabel).alignRightEdgeTo(intensityLabel);
	mui::L(afterglowSlider).rightOf(afterglowLabel,5).stretchToRightEdgeOf(this,10);
	
	
	height = afterglowSlider->y + afterglowSlider->height;
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
	
	static float lastStrokeWeightVal = -1;
	if( !updateSlider(strokeWeightSlider, globals.strokeWeight, lastStrokeWeightVal ) ){
		globals.strokeWeight = strokeWeightSlider->value;
	}
	
	static float lastBlurVal = -1;
	if( !updateSlider(blurSlider, globals.blur, lastBlurVal ) ){
		globals.blur = blurSlider->value;
	}
	
	static float lastNumPtsVal = -1;
	if( !updateSlider(numPtsSlider, globals.numPts, lastNumPtsVal ) ){
		globals.numPts = numPtsSlider->value;
	}
	
	static float lastHueVal = -1;
	if( !updateSlider(hueSlider, globals.hue, lastHueVal ) ){
		globals.hue = hueSlider->value;
	}
	
	static float lastIntensityVal = -1;
	if( !updateSlider(intensitySlider, globals.intensity, lastIntensityVal ) ){
		globals.intensity = intensitySlider->value;
	}
	
	static float lastAfterglowVal = -1;
	if( !updateSlider(afterglowSlider, globals.afterglow, lastAfterglowVal ) ){
		globals.afterglow = afterglowSlider->value;
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

mui::Label * OsciView::addLabel( string text ){
	mui::Label * label = new mui::Label( text, 0, 0, 100, 30 );
	label->fontSize = 10;
	label->commit();
	label->width = label->boundingBox.width;
	label->horizontalAlign = mui::Right;
	add( label );
	return label;
}

