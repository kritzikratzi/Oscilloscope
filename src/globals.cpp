//
//  globals.cpp
//  Oscilloscope
//
//  Created by Hansi on 26.07.15.
//
//

#include "globals.h"


Globals Globals::instance; 

string ofxFormatTime(double seconds) {
	int total = seconds; 
	int s = total % 60; 
	
	total = total / 60; 
	int m = total % 60; 

	total = total / 60; 
	int h = total; 

	return ofToString(h, 2, '0') + ":" + ofToString(m, 2, '0') + ":" + ofToString(s, 2, '0'); 
}