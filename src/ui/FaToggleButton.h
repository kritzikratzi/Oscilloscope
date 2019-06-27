//
//  FAButton.h
//  OsciStudio
//
//  Created by Hansi on 04.04.15.
//
// 20.12.2018: added optional fontName argument. this makes it easy to use with multiple versions of fontawesome


#ifndef FaToggleButton_h
#define FaToggleButton_h

#include "ofxMightyUI.h"
#include "ofMain.h"

class FaToggleButton : public mui::ToggleButton{
public:
	FaToggleButton( string icon, string selectedIcon, float x = 0, float y = 0, float width = 100, float height = 20, const string & fontName = "fontawesome-webfont.ttf" ) :
	mui::ToggleButton(icon, x, y, width, height ), wasSelected(false){
		this->icon = icon;
		this->selectedIcon = selectedIcon;
		this->label->fontName = fontName;
		this->label->commit();
		this->fg = ofColor(255); 
		
		ofAddListener( onPress, this, &FaToggleButton::onButtonPressed );
	}
	
	string icon;
	string selectedIcon;
	
	bool wasSelected;
	
	void commit(){
		if( wasSelected != selected ){
			wasSelected = selected;
			label->text = selected? selectedIcon:icon;
			label->commit();
		}
	}
	
private:
	void onButtonPressed( const void * sender, ofTouchEventArgs &touch ){
		commit();
	}
};
#endif
