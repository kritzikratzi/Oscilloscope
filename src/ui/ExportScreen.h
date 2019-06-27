//
//  ExportScreen.hpp
//  Oscilloscope
//
//  Created by Hansi on 29.05.18.
//
//

#ifndef ExportScreen_hpp
#define ExportScreen_hpp

#include "ofMain.h"
#include "MuiCore.h"

class ofxAvVideoWriter;

enum class ExportFormat;

class ExportScreen : public mui::Container{
public:
	ExportScreen();
	~ExportScreen();
	
	void show(const ofFile & file); 
	void layout() override; 
	
	const std::filesystem::path & getFile();
	ExportFormat getFormat();
private:
	
	void buttonPressed(const void * sender, ofTouchEventArgs & args );
	
	mui::Container * view;
	
	mui::Label * filePickerLabel;
	mui::Label * formatSelectLabel;
	mui::Label * sizeLabel;
	mui::Label * sizeXLabel;
	mui::Label * sizeAtLabel;
	mui::Label * sizeFpsLabel;
	
	mui::TextArea * widthText;
	mui::TextArea * heightText;
	mui::TextArea * fpsText; 
	
	mui::SegmentedSelect<ExportFormat> * formatSelect;
	
	shared_ptr<ofxAvVideoWriter> writer;
	mui::FilePicker * filePicker;
	mui::Button * exportButton;
	mui::Button * cancelButton;
	mui::Button * resetButton;
	
	string defaultName; 
};



#endif /* ExportScreen_hpp */
