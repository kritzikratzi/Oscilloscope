#include "ofxMightyUI.h"
#include "Dialogs.h"
#include "MuiL.h"

Dialog::Dialog() : mui::Container(0,0,400,100){
	blocker = new mui::Container();
	blocker->onLayout.add([&](){
		blocker->x = 0;
		blocker->y = 0;
		blocker->width = blocker->parent->width;
		blocker->height = blocker->parent->height;
	});
	blocker->bg = ofColor(0,200);
	blocker->opaque = true;
	
	bg = ofColor(0);
	opaque = true;
}

Dialog::~Dialog(){
	if(blocker) MUI_ROOT->safeRemoveAndDelete(blocker);
	
	for(mui::Container * btn : buttons){
		delete btn;
	}
	for(mui::Container * lbl : labels){
		delete lbl;
	}

	buttons.clear();
	labels.clear();

	children.clear();
}

void Dialog::layout(){
	mui::L(labels).rows({10,10}).stretchToRightEdgeOfParent(10);
	
	float btn_y0 = 20 + (labels.size() == 0? 0:labels.back()->getBounds().getBottom());
	
	mui::L(buttons).columns({10,btn_y0});
	
	height = getChildBounds().getBottom() + 10;
	
	if(parent){
		x = (parent->width - width)/2;
		y = (parent->height - height)/2;
	}
}

void Dialog::draw(){
	ofNoFill();
	ofDrawRectangle(0,0,width-1,height-1);
	ofFill();
}

void Dialog::show(){
	MUI_ROOT->add(blocker);
	blocker->handleLayout();

	MUI_ROOT->add(this);
	layout();
}

void Dialog::close(){
	blocker->visible = false;
	MUI_ROOT->safeRemoveAndDelete(blocker);
	blocker = nullptr;
	
	visible = false;
	MUI_ROOT->safeRemoveAndDelete(this);
}

mui::Label * Dialog::addLabel(const std::string & text){
	mui::Label * label = new mui::Label(text,0,0,380,20);
	add(label);
	labels.push_back(label);
}

mui::Button * Dialog::addButton(const std::string & text, const std::function<void()> & on_choice){
	mui::Button * button = new mui::Button(text,0,0,90,30);
	button->setProperty("on_choice", on_choice);
	ofAddListener(button->onPress, this, &Dialog::buttonPressed);
	add(button);
	buttons.push_back(button);
}

void Dialog::buttonPressed(const void * sender, ofTouchEventArgs & args){
	for(mui::Container * btn : buttons){
		if(btn == sender){
			std::function<void()> * func = btn->getProperty<std::function<void()>>("on_choice");
			close();
			if(func && *func) (*func)();
		}
	}
}


Dialog * show_yes_no_dialog(const std::string & message, const std::function<void(bool)>& on_choice){
	Dialog * d = new Dialog();
	d->addLabel(message);
	d->addButton("Yes", [on_choice](){ on_choice(true); });
	d->addButton("No", [on_choice](){ on_choice(false); });
	d->show();
	return d;
}

Dialog * show_ok_dialog(const std::string & message, const std::function<void()>& on_choice){
	Dialog * d = new Dialog();
	d->addLabel(message);
	d->addButton("Ok", [on_choice](){ on_choice(); });
	d->show();
	return d;
}
