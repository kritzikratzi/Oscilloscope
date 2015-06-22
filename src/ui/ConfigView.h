#pragma once

#include "MUI.h"
#include "../settings.h"


class ConfigView : public mui::Container{
public:
	ConfigView( float x_ = 0, float y_ = 0, float width_ = ofGetWidth(), float height_ = ofGetHeight() );
	
	virtual void update();
	virtual void draw();
	virtual void drawBackground();
	
	virtual void touchDown( ofTouchEventArgs &touch );
	virtual void touchMoved( ofTouchEventArgs &touch );
	virtual void touchUp( ofTouchEventArgs &touch );
	virtual void touchDoubleTap( ofTouchEventArgs &touch );
	
	
	mui::SegmentedSelect * sampleRatesSelect;
	mui::SegmentedSelect * bufferSizeSelect;
	mui::SegmentedSelect * numbuffersSelect;
	mui::Button * startButton;
	
	void fromSettings( Settings & settings );
	void toSettings( Settings & settings );
	
	void buttonPressed( const void * sender, ofTouchEventArgs & args ); 
	void selectSoundCard( mui::ToggleButton * card );
	
private:
	
	void pushLabel( string text, float &x, float &y, float &w, float &h );
	
	int selectedSoundCard;
	vector<mui::ToggleButton*> soundcardButtons;
};
