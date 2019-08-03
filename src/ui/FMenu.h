/*
*  FMenu.h
*  ofxMightyUI
*
*  v1.14, add button with custom callback
*  v1.13, set minWidth on init
*  v1.12, removed highlightSelected option from dropdown
*  v1.11, add removeAllOptionsAndNotify()
*  v1.10, renamed dropdown::getOptionByName to dropdown::getOptionByValue (which is what was meant in the first place)
*  v1.9, added getOptionByValue(...), cleaned up a tiny bit
*  v1.8, undocumented.... minWidth, fixed layout issues, fontsize back to normal
*  v1.7, fdropdown: triangle scale, data display function, use mui_root->popup functions for dropdown
*  v1.6, merged 1.3a-1.5 from AL, merged 1.3-1.5 from OS
*  v1.5, allow multicolumn, rewrite layouting
*  v1.4, mouse over effect for buttons
*  v1.3a, add remove all options method
*  v1.5, 21.11.2016, removed funny dependency to soundythingie
*  v1.4, dropdown class now has a "datadelegate"
*  v1.3, added a dropdown button class with embedded menu
*  v1.2, it actually sortof works a bit, with the options.
*  v1.1, ability to attach data to an option
*  v1.0, 29.01.11. initial version
*
*/

#pragma once

#include "ofxMightyUI.h"

enum class FMenuNotify{
	always, if_changed, never
};

template<typename T>
class FMenu : public mui::Container {
public:
	struct Option {
		Option(mui::Button * b, const T & v, mui::EventHandler<Option> onSel) : button(b), value(v), onSelect(onSel) {}
		mui::Button * button;
		T value;
		mui::EventHandler<Option> onSelect;
	};

	FMenu(float x_ = 0, float y_ = 0, float width_ = 200, float height_ = 20);
	~FMenu();

	mui::Container * view;
	ofEvent<Option> onSelectOption;
	ofEvent<T> onSelectValue;
	ofEvent<void> onDeselect;

	Option * selected;
	bool highlightActiveItem{ false };

	Option * addOption(string label, T value);
	template<typename Callback>
	Option * addOption(string label, T value, Callback c) {
		auto option = addOption(label, value); 
		option->onSelect.add(c); 
		return option; 
	}; 
	Option * getOption(int num);
	Option * getOptionByValue(const T & value);
	typename FMenu<T>::Option * setSelected(Option * option, FMenuNotify when = FMenuNotify::always);
	typename FMenu<T>::Option * setSelectedValue(T value, FMenuNotify when = FMenuNotify::always);
	typename FMenu<T>::Option * setSelectedIndex(int index, FMenuNotify when = FMenuNotify::always);


	void removeAllOptions(FMenuNotify when = FMenuNotify::always);
	size_t getNumOptions();
	void addSeparator();

	void layout() override;
	void handleDraw() override;

	void setMinWidth(float minWidth);
	float getMinWidth();

	void buttonPressed(const void * sender, ofTouchEventArgs &args);

	vector<Option*> items;
	bool allowMultiColumn{ true };
	int maxRowsPerColumn{ 15 };
	float columnWidth{ 0 };
	float columnPadding{ 1 };
	float topBorderWidth{ -1 }; // -1 = full width, -2=don't draw, >=0 means border inset from left
	string defaultText; 
private:
	unsigned long long lastTouchTime;
	ofVec2f lastTouchPos;
	float posY;
	float posX;
	float minWidth = 0;
	int currentNumColumns{ 0 };
	float currentColumnWidth{ 200 };
	Option * lastOption = nullptr; 

	template<typename U> friend class FDropdown;
};


template<typename T>
class FDropdown : public mui::Container {
public:
	FDropdown(float x_ = 0, float y_ = 0, float width_ = 200, float height_ = 20);
	~FDropdown();

	mui::Label * label;
	bool pressed;
	float triangleScale = 1.0;

	ofEvent<ofTouchEventArgs> onPress;
	ofEvent<typename FMenu<T>::Option> onSelectOption;
	ofEvent<T> onSelectValue;

	function<void(FDropdown*)> dataProvider{ nullptr };
	function<string(string, T)> dataDisplay{ nullptr };

	void deselect();
	typename FMenu<T>::Option * addOption(string label, T value);
	typename FMenu<T>::Option * getOption(int num);
	typename FMenu<T>::Option * getOptionByValue(const T & value);
	size_t getNumOptions();

	void setDefaultText(const string & defaultText); 
	const string & getDefaultText() const; 

	typename FMenu<T>::Option * getSelected() const;
	const T & getSelectedValueOr(const T & defaultValue) const;
	typename FMenu<T>::Option * setSelected(typename FMenu<T>::Option * option, FMenuNotify when = FMenuNotify::always);
	typename FMenu<T>::Option * setSelectedValue(T value, FMenuNotify when = FMenuNotify::always);
	typename FMenu<T>::Option * setSelectedIndex(int index, FMenuNotify when = FMenuNotify::always);

	void removeAllOptions(FMenuNotify when = FMenuNotify::always);

	void update() override;
	void draw() override;
	void drawBackground() override;
	void fitWidthToLabel(int paddingLR = 5);
	void fitWidthToLabel(int paddingL, int paddingR);
	void layout() override;

	// simulate a click
	void clickAndNotify();

	void touchDown(ofTouchEventArgs &touch) override;
	void touchMoved(ofTouchEventArgs &touch) override;
	void touchMovedOutside(ofTouchEventArgs &touch) override;
	void touchUp(ofTouchEventArgs &touch) override;
	void touchUpOutside(ofTouchEventArgs &touch) override;
	void touchDoubleTap(ofTouchEventArgs &touch) override;
	void touchCanceled(ofTouchEventArgs &touch) override;



	FMenu<T> * menu;

private:
	string defaultText = "-";
	void updateDisplayText();
	void optionSelected(const void * sender, typename FMenu<T>::Option &option);
	ofMesh triangle;
	bool needsUpdate = false; 
};
