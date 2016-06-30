//
//  L.h
//  Oscilloscope
//
//  Created by Hansi on 26.07.15.
//
//
// tested this with the oscilloscope,
// might move to mui if it turns out handy!
// thats why it's already in the mui namespace
#ifndef mui_L_h
#define mui_L_h

#include "ofxMightyUI.h"

namespace mui{
	class ${
	public:
		$( mui::Container * target ) : target(target){
		}

		$ & pos( float x, float y ){
			target->x = x;
			target->y = y;
			return *this;
		}
		
		mui::Container * target;
	};
	
	
	class L{
	public:
		L( mui::Container * target ) : target(target){
		}
		
		L & pos( float x, float y ){
			target->x = x;
			target->y = y;
			return *this;
		}
		
		L & rightOf( mui::Container * dest, float space = 0 ){
			target->x = dest->x + dest->width + space;
			target->y = dest->y;
			return *this;
		}
		
		L & leftOf( mui::Container * dest, float space = 0 ){
			target->x = dest->x - target->width - space;
			target->y = dest->y;
			return *this;
		}
		
		L & alignLeftEdgeTo( mui::Container * dest, float space = 0, mui::HorizontalAlign align = mui::Left ){
			if( align == mui::Left ){
				target->x = dest->x + space;
			}
			else if( align == mui::Right ){
				target->x = dest->x - target->width - space;
			}
			else if( align == mui::Center ){
				target->x = dest->x + dest->width/2 + space;
			}
			
			return *this;
		}
		
		L & alignRightEdgeTo( mui::Container * dest, float space = 0, mui::HorizontalAlign align = mui::Right ){
			if( align == mui::Left ){
				target->x = dest->x - target->width + space;
			}
			else if( align == mui::Right ){
				target->x = dest->x + dest->width - target->width - space;
			}
			else if( align == mui::Center ){
				target->x = dest->x + dest->width/2 - target->width + space;
			}
			
			return *this;
		}
		
		L & maxWidth( float width ){
			target->width = min(width, target->width);
			return *this; 
		}
		
		L & stretchToRightEdgeOf( mui::Container * dest, float space = 0 ){
			// todo: this assumes dest is the parent,
			// it doesn't really have to be! 
			target->width = dest->width - space - target->x;
			return * this;
		}
		
		L & below( mui::Container * dest, float space = 0 ){
			target->x = dest->x;
			target->y = dest->y + dest->height + space;
			return *this; 
		}
		
		mui::Container * target;
	};
}

#endif
