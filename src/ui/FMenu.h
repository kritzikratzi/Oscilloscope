/*
 *  FMenu.h
 *  iPhoneEmptyExample
 *
 *  Use this as a starting point for a new element!
 *
 *  Created by hansi on 29.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "MUI.h"

class FMenu : public mui::Container{
public:
	FMenu( float x_ = 0, float y_ = 0, float width_ = 200, float height_ = 20 );
	~FMenu();
	
	mui::Container * view;
	ofEvent<ofTouchEventArgs> onPress;
	
	virtual mui::Button * addButton( string label );
	virtual void addSeparator();
	
	virtual void autoSize( mui::Direction dir = mui::Both );
	virtual ofRectangle getViewBoundingBox();
	virtual void update();
	virtual void draw();
	virtual void touchDown( ofTouchEventArgs &touch );
	virtual void touchMoved( ofTouchEventArgs &touch );
	virtual void touchMovedOutside( ofTouchEventArgs &touch );
	virtual void touchUp( ofTouchEventArgs &touch );
	virtual void touchDoubleTap( ofTouchEventArgs &touch );
	virtual void buttonPressed( const void * sender, ofTouchEventArgs &args ); 
	
private:
	unsigned long long lastTouchTime;
	ofVec2f lastTouchPos;
	vector<mui::Container*> items;
	float posY;
};

