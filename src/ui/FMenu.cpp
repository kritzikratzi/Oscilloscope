#include "FMenu.h"

using namespace mui;


#pragma mark FMenuButton

template<typename T>
class FMenuButton : public mui::Button {

public:

	FMenuButton(FMenu<T> * menu, string title, float x, float y, float w, float h) : menu(menu), mui::Button(title, x, y, w, h) {
		label->x += 5; // some padding!
		bg = ofColor(0, 0);
	}

	void layout() override {
		label->x = 5;
		float w = width - 10; 
		if (label->width != w) {
			label->width = w; 
			label->commit();
		}
	}

	void drawBackground() override {
		if (isMouseOver()) {
			bg = ofColor(75); 
		}
		else {
			bg = (menu->selected != nullptr && menu->selected->button == this && menu->highlightActiveItem) ? ofColor(50) : ofColor(0);
		}
		mui::Button::drawBackground(); 
	}

private:
	FMenu<T> * menu;
};

#pragma mark FMenu

template<typename T>
FMenu<T>::FMenu(float x_, float y_, float width_, float height_) : Container(x_, y_, width_, height_), lastTouchTime(0), posY(0), selected(NULL) {
	view = new Container(2, 2, width - 4, height - 4);
	view->ignoreEvents = true;
	view->name = "FMenu-view";
	opaque = true;
	bg = ofColor(0);
	minWidth = width; 
	add(view);
};

template<typename T>
FMenu<T>::~FMenu() {
	delete view;
	for (Option * item : items) {
		delete item->button;
	}
	items.clear();
}


//--------------------------------------------------------------
template<typename T>
typename FMenu<T>::Option * FMenu<T>::addOption(string title, T value) {
	Button * button = new FMenuButton<T>(this, title, posX, posY, view->width, 22);
	//button->label->fontSize -= 2;
	button->label->commit();
	ofAddListener(button->onPress, this, &FMenu<T>::buttonPressed);
	button->label->horizontalAlign = mui::Left;
	posY += 22 + 1;
	view->add(button);
	layout();

	Option * option = new Option(button, value, mui::EventHandler<Option>(button));

	items.push_back(option);

	return option;
}

template<typename T>
void FMenu<T>::removeAllOptions(FMenuNotify when) {
	posY = 0;
	for (Option * option : items) {
		view->remove(option->button);
		delete option->button;
		delete option;
	}
	items.clear();
	handleLayout();

	bool same = selected == nullptr; 
	selected = nullptr; 

	bool notify = (!same && when == FMenuNotify::if_changed) || when == FMenuNotify::always;
	if (notify) {
		onDeselect.notify(this);
	}

}

template<typename T>
typename FMenu<T>::Option * FMenu<T>::getOption(int num) {
	if (num >= 0 && num < items.size()) {
		return items[num];
	}
	else {
		return nullptr;
	}
}

template<typename T>
typename FMenu<T>::Option * FMenu<T>::getOptionByValue(const T & value) {
	for (auto item : items) {
		if (item->value == value) {
			return item; 
		}
	}

	return nullptr; 
}

template<typename T>
typename FMenu<T>::Option * FMenu<T>::setSelected(typename FMenu<T>::Option * option, FMenuNotify when) {
	bool same =
		(option == nullptr && selected != nullptr) ||
		(option != nullptr && selected == nullptr) ||
		(selected != nullptr && option->value == selected->value);

	selected = option; 

	bool notify = (!same && when == FMenuNotify::if_changed) || when==FMenuNotify::always;
	if (notify) {
		onSelectOption.notify(this, *option);
		onSelectValue.notify(this, option->value);
		option->onSelect.notify(this, *option); 
	}

	return option; 
}

template<typename T>
typename FMenu<T>::Option * FMenu<T>::setSelectedValue(T value, FMenuNotify when ) {
	for (auto & item : items) {
		if (item->value == value) {
			return setSelected(item, when); 
		}
	}

	return nullptr;
}

template<typename T>
typename FMenu<T>::Option * FMenu<T>::setSelectedIndex(int index, FMenuNotify when) {
	if (items.size() == 0) {
		return setSelected(nullptr, when);
	}
	else {
		return setSelected(items[MAX(0, MIN(index, items.size() - 1))]);
	}
}



template<typename T>
size_t FMenu<T>::getNumOptions() {
	return items.size();
}

template<typename T>
void FMenu<T>::setMinWidth(float minWidth) {
	needsLayout |= this->minWidth != minWidth;
	this->minWidth = minWidth;
}

template<typename T>
float FMenu<T>::getMinWidth() {
	return minWidth;
}


template<typename T>
void FMenu<T>::addSeparator() {
	// fuck separators!!! 
	// posY += BUTTON_SEPARATE;
	/*mui::Label * label = new mui::Label( "-----------------", 0, posY, view->width, BUTTON_HEIGHT/2 );
	posY += BUTTON_HEIGHT;
	view->add( label );
	autoSize( mui::Vertical );

	items.push_back( label );

	return label;*/
}

template<typename T>
void FMenu<T>::handleDraw() {
	mui::Container::handleDraw();
	if (visible) {
		if (currentNumColumns > 0) {
			ofSetColor(75);
			for (int i = 1; i < currentNumColumns; i++) {
				float x0 = x + view->x + i*currentColumnWidth + (i - 1)*columnPadding + 0.5;
				float y0 = y + view->y;

				ofDrawLine(x0, y0, x0, y0 + view->height);
			}
			ofSetColor(255);
		}
	}

	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
	mesh.addVertex({ x, y, 0 });
	mesh.addVertex({ x, y + height - 1, 0 });
	mesh.addVertex({ x + width - 1, y + height - 1, 0 });
	mesh.addVertex({ x + width - 1, y, 0 });
	if(topBorderWidth==-1) mesh.addVertex({ x, y, 0 });
	else if (topBorderWidth >= 0) mesh.addVertex({x + topBorderWidth, y, 0});

	ofPushMatrix();
	ofTranslate(.5f, 0.5f);
	mesh.draw();
	ofPopMatrix();
}

//--------------------------------------------------------------
template<typename T>
void FMenu<T>::layout() {
	float posX = 0;
	float posY = 0;
	float h = 0;
	float padx = 4;
	float pady = 4;

	if (allowMultiColumn) {
		int i = 0;
		currentNumColumns = 0;

		int nc = ceilf(view->children.size() / (float)max(1, maxRowsPerColumn));
		float w = columnWidth > 0 ? columnWidth : ((minWidth>0 ? minWidth : 200) - 2 * padx);
		float cw = w * nc + columnPadding*(nc - 1);
		float tw = max(minWidth, cw + 2 * padx);

		currentColumnWidth = (tw - 2 * padx - columnPadding*(nc - 1)) / nc;


		for (mui::Container * child : view->children) {
			if (i>maxRowsPerColumn) {
				posY = 0;
				posX += currentColumnWidth + columnPadding;
				i = 0;
				currentNumColumns++;
			}
			child->setBounds(posX, posY, currentColumnWidth, child->height);
			posY += child->height + 1;
			h = max(posY - 1, h);
			i++;
		}
	}
	else {
		float w = columnWidth > 0 ? columnWidth : ((minWidth>0 ? minWidth : 200) - 2 * padx);
		currentNumColumns = 0;
		currentColumnWidth = max(w, minWidth - 2 * padx);
		for (mui::Container * child : view->children) {
			child->setBounds(posX, posY, currentColumnWidth, child->height);
			posY += child->height + 1;
			h = max(posY, h);
		}
	}

	view->x = padx;
	view->y = padx;
	view->width = posX + currentColumnWidth;
	view->height = h;
	width = view->width + 2 * padx;
	height = view->height + 2 * pady;
}

//--------------------------------------------------------------
template<typename T>
void FMenu<T>::buttonPressed(const void * sender, ofTouchEventArgs &args) {
	for (Option * item : items) {
		if (item->button == sender) {
			setSelected(item,FMenuNotify::always);
		}
	}
}


#pragma mark FDropdown


#include "Button.h"
#include "MuiCore.h"
#include "Label.h"

using namespace mui;

//--------------------------------------------------------------
template<typename T>
FDropdown<T>::FDropdown(float x_, float y_, float width_, float height_) : Container(x_, y_, width_, height_), pressed(false) {
	label = new Label("-", 0, 0, width, height);
	label->horizontalAlign = Left;
	label->verticalAlign = Middle;
	label->fg.r = label->fg.g = label->fg.b = 255;
	label->fontSize = mui::MuiConfig::fontSize;
	label->ellipsisMode = true;
	label->commit();
	bg = ofColor(128, 50);
	add(label);
	opaque = true;
	name = "some-fdropdown";
	needsUpdate = true; 

	menu = new FMenu<T>();
	menu->columnWidth = 0;
	menu->topBorderWidth = -2; 
	menu->highlightActiveItem = true; 
	ofAddListener(menu->onSelectOption, this, &FDropdown<T>::optionSelected);
}

template<typename T>
FDropdown<T>::~FDropdown() {
	MUI_ROOT->safeRemoveAndDelete(menu);
}

template<typename T>
typename FMenu<T>::Option * FDropdown<T>::addOption(string label, T value) {
	return menu->addOption(label, value);
}

template<typename T>
typename FMenu<T>::Option * FDropdown<T>::getOption(int num) {
	return menu->getOption(num);
}

template<typename T>
typename FMenu<T>::Option * FDropdown<T>::getOptionByValue(const T & value) {
	return menu->getOptionByValue(value); 
}


template<typename T>
size_t FDropdown<T>::getNumOptions() {
	return menu->getNumOptions();
}

template<typename T>
void FDropdown<T>::setDefaultText(const string & defaultText) {
	this->defaultText = defaultText; 
	updateDisplayText(); 
}

template<typename T>
const string & FDropdown<T>::getDefaultText() const {
	return defaultText; 
}


template<typename T>
typename FMenu<T>::Option * FDropdown<T>::getSelected() const {
	return menu->selected;
}

template<typename T>
const T & FDropdown<T>::getSelectedValueOr(const T & defaultValue) const {
	auto sel = getSelected();
	return sel ? sel->value : defaultValue;
}

template<typename T>
typename FMenu<T>::Option * FDropdown<T>::setSelected(typename FMenu<T>::Option * option, FMenuNotify when) {
	needsUpdate = true; 
	return menu->setSelected(option, when);
}

template<typename T>
typename FMenu<T>::Option * FDropdown<T>::setSelectedValue(T value, FMenuNotify when ) {
	needsUpdate = true;
	return menu->setSelectedValue(value, when);
}

template<typename T>
typename FMenu<T>::Option * FDropdown<T>::setSelectedIndex(int index, FMenuNotify when) {
	needsUpdate = true;
	return menu->setSelectedIndex(index, when);
}

template<typename T>
void FDropdown<T>::removeAllOptions(FMenuNotify when) {
	needsUpdate = true;
	menu->removeAllOptions(when);
}




//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::update() {
	label->x = 5;
	label->width = width - 17;
	label->height = height;

	if (needsUpdate) {
		updateDisplayText(); 
		needsUpdate = false; 
	}
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::draw() {
	ofPushMatrix();
	ofTranslate(width - 7, height / 2);
	ofScale(triangleScale, triangleScale, 1);
	if (menu->parent != nullptr) {
		ofDrawTriangle(-5, 3, 5, 3, 0, -3);
	}
	else {
		ofDrawTriangle(-5, -3, 5, -3, 0, 3);
	}
	ofPopMatrix();
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::drawBackground() {
	if (pressed) {
		ofSetColor(bg.r / 2, bg.g / 2, bg.b / 2, bg.a);
	}
	else {
		ofSetColor(bg.r, bg.g, bg.b, bg.a);
	}
	ofDrawRectangle(0, 0, width, height);
	ofSetColor(255);
}

template<typename T>
void FDropdown<T>::fitWidthToLabel(int paddingLR) {
	fitWidthToLabel(paddingLR, paddingLR);
}

template<typename T>
void FDropdown<T>::fitWidthToLabel(int paddingL, int paddingR) {
	label->ellipsisMode = false;
	label->commit();
	ofRectangle bounds = label->box(0, paddingR, 0, paddingL);
	width = bounds.width;
}

template<typename T>
void FDropdown<T>::layout() {
	label->width = width;
	label->height = height;
}

//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::touchDown(ofTouchEventArgs &touch) {
	pressed = true;
	if (menu->parent && menu->visible) {
		MUI_ROOT->removePopup(menu);
	}
	else {
		if (dataProvider != nullptr) {
			T oldValue = menu->selected == nullptr ? T() : menu->selected->value;
			removeAllOptions(FMenuNotify::never);
			dataProvider(this);
			setSelectedValue(oldValue, FMenuNotify::if_changed);
		}
		ofVec2f pos = getGlobalPosition();
		menu->setMinWidth(width);
		MUI_ROOT->showPopupMenu(menu, this, 0, height);
	}
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::touchMoved(ofTouchEventArgs &touch) {
	pressed = true;
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::touchMovedOutside(ofTouchEventArgs &touch) {
	pressed = false;
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::touchUp(ofTouchEventArgs &touch) {
	pressed = false;
	ofNotifyEvent(onPress, touch, this);
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::touchUpOutside(ofTouchEventArgs &touch) {
	pressed = false;
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::touchDoubleTap(ofTouchEventArgs &touch) {
}


//--------------------------------------------------------------
template<typename T>
void FDropdown<T>::touchCanceled(ofTouchEventArgs &touch) {
	pressed = false;
}

template<typename T>
void FDropdown<T>::clickAndNotify() {
	ofTouchEventArgs args;
	args.x = width / 2;
	args.y = height / 2;
	ofNotifyEvent(onPress, args, this);
}

template<typename T>
void FDropdown<T>::updateDisplayText() {
	if (menu->selected == nullptr) {
		if (dataDisplay) label->text = dataDisplay("", T());
		else label->text = defaultText;
	}
	else {
		if (dataDisplay) label->text = dataDisplay(menu->selected->button->label->text, menu->selected->value);
		else label->text = menu->selected->button->label->text;
	}
	label->commit();
}

template<typename T>
void FDropdown<T>::optionSelected(const void * sender, typename FMenu<T>::Option &option) {
	updateDisplayText(); 
	onSelectOption.notify(this, option);
	onSelectValue.notify(this, option.value);
	option.onSelect.notify(this, option); 

	MUI_ROOT->removePopup(menu);
}

template<typename T>
void FDropdown<T>::deselect() {
	label->text = defaultText;
	label->commit();
	MUI_ROOT->removePopup(menu);
}

template class FMenu<int>;
template class FMenu<float>;
template class FMenu<double>;
template class FMenu<string>;
template class FDropdown<int>;
template class FDropdown<float>;
template class FDropdown<double>;
template class FDropdown<string>;
