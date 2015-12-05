#include "FMenu.h"


FMenu::FMenu( float x_, float y_, float width_, float height_ ) : Container( x_, y_, width_, height_ ), lastTouchTime(0), posY(0){
	view = new Container( 2, 2, width - 4, height - 4 );
	view->ignoreEvents = true;
	view->name = "FMenu-view";
	opaque = true;
	add( view );
};

FMenu::~FMenu(){
	delete view;
	vector<mui::Container*>::iterator it = items.begin();
	while( it != items.end() ){
		delete *it;
		++it;
	}
	items.clear();
}

//--------------------------------------------------------------
void FMenu::update(){
}


//--------------------------------------------------------------
void FMenu::draw(){
}

//--------------------------------------------------------------
mui::Button * FMenu::addButton( string title ){
	mui::Button * button = new mui::Button( title, 0, posY, view->width, 22 );
	button->label->fontSize -= 2;
	button->label->commit();
	ofAddListener(button->onPress, this, &FMenu::buttonPressed);
	button->label->horizontalAlign = mui::Left;
	posY += 22 + 1;
	view->add( button );
	autoSize( mui::Vertical );
	items.push_back( button );
	
	return button;
}

void FMenu::addSeparator(){
	// fuck separators!!! 
	// posY += BUTTON_SEPARATE;
	/*mui::Label * label = new mui::Label( "-----------------", 0, posY, view->width, BUTTON_HEIGHT/2 );
	posY += BUTTON_HEIGHT;
	view->add( label );
	autoSize( mui::Vertical );
	
	items.push_back( label );
	
	return label;*/
}

//--------------------------------------------------------------
//--------------------------------------------------------------
ofRectangle FMenu::getViewBoundingBox(){
	// figure out min/max values...
	std::vector<Container*>::iterator it = view->children.begin();
	float minX, minY, maxX, maxY;
	
	minX = 0;
	minY = 0;
	maxX = 0;
	maxY = 0;
	
	while( it != view->children.end() ) {
		minX = fminf( (*it)->x, minX );
		minY = fminf( (*it)->y, minX );
		maxX = fmaxf( (*it)->x + (*it)->width, maxX );
		maxY = fmaxf( (*it)->y + (*it)->height, maxY );
		++it;
	}
	
	return ofRectangle( minX, minY, maxX - minX, maxY - minY );
}

//--------------------------------------------------------------
void FMenu::autoSize( mui::Direction dir ){
	ofRectangle boundingBox = getViewBoundingBox();
	if( dir & mui::Horizontal ){
		view->width = boundingBox.width;
		width = view->width + 4;
	}
	if( dir & mui::Vertical ){
		view->height = boundingBox.height;
		height = view->height + 4;
	}
}
//--------------------------------------------------------------
void FMenu::touchDown( ofTouchEventArgs &touch ){
}

//--------------------------------------------------------------
void FMenu::touchMoved( ofTouchEventArgs &touch ){
}

//--------------------------------------------------------------
void FMenu::touchMovedOutside( ofTouchEventArgs &touch ){
	touchMoved( touch );
}

//--------------------------------------------------------------
void FMenu::touchUp( ofTouchEventArgs &touch ){
}

//--------------------------------------------------------------
void FMenu::touchDoubleTap( ofTouchEventArgs &touch ){
}

//--------------------------------------------------------------
void FMenu::buttonPressed( const void * sender, ofTouchEventArgs &args ){
	onPress( sender, args );
}
