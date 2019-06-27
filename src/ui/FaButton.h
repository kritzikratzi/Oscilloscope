//
//  FAButton.h
//  OsciStudio
//
//  Created by Hansi on 04.04.15.
//
// 20.12.2018: added optional fontName argument. this makes it easy to use with multiple versions of fontawesome

#ifndef FaButton_h
#define FaButton_h

#include "ofxMightyUI.h"

class FaButton : public mui::Button{
public:
	FaButton( string icon, float x = 0, float y = 0, float width = 100, float height = 20, const string & fontName = "fontawesome-webfont.ttf" ) :
	mui::Button(icon, x, y, width, height ){
		this->label->fontName = fontName;
		this->label->commit();
	}
};
#endif
