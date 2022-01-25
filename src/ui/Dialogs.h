#pragma once

#include "MuiCore.h"
#include <functional>

class Dialog : public mui::Container{
public:
	Dialog();
	virtual ~Dialog();
	
	void layout() override;
	void draw() override;
	
	void show();
	void close();
	
	mui::Label * addLabel(const std::string & text);
	mui::Button * addButton(const std::string & text, const std::function<void()> & on_choice);
private:
	void buttonPressed(const void * sender, ofTouchEventArgs & args); 

	mui::Label * label;
	std::vector<mui::Container *> buttons;
	std::vector<mui::Container *> labels;
	mui::Container * blocker;
};

Dialog * show_yes_no_dialog(const std::string & message, const std::function<void(bool)> & on_choice);
Dialog * show_ok_dialog(const std::string & message, const std::function<void()> & on_choice);
