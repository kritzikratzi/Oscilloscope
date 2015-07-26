//
//  FAButton.h
//  OsciStudio
//
//  Created by Hansi on 04.04.15.
//
//

#ifndef FaButton_h
#define FaButton_h

#include "MUI.h"

class FaButton : public mui::Button{
public:
	FaButton( string icon, float x = 0, float y = 0, float width = 100, float height = 20 ) :
	mui::Button(icon, x, y, width, height ){
		this->label->fontName = "fontawesome-webfont.ttf";
		this->label->commit();
	}
	
};
#endif
