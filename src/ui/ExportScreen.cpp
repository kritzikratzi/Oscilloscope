//
//  ExportScreen.cpp
//  Oscilloscope
//
//  Created by Hansi on 29.05.18.
//
//

#include "ExportScreen.h"
#include "MuiL.h"
#include "MuiFilePicker.h"
#include "MuiTextArea.h"
#include "globals.h"
#include "Dialogs.h"

ExportScreen::ExportScreen() : mui::Container(){
	
	opaque = true;
	bg = ofColor(0,150);
	
	view = new mui::Container();
	view->name = "export-view";
	view->opaque = true;
	view->bg = ofColor(30);
	add(view);
	
	
	auto makeLabel = [&](string text){
		mui::Label * label = new mui::Label(text, 0, 0, 100, 30);
		view->add(label);
		return label;
	};
	
	exportButton = new mui::Button("Export",0,0,100,30);
	exportButton->bg = ofColor(255,0,0);
	exportButton->fg = ofColor(255);
	ofAddListener(exportButton->onPress, this, &ExportScreen::buttonPressed);
	view->add(exportButton);
	
	resetButton = new mui::Button("Reset",0,0,100,30);
	ofAddListener(resetButton->onPress, this, &ExportScreen::buttonPressed);
	view->add(resetButton);
	
	cancelButton = new mui::Button("Cancel",0,0,100,30);
	ofAddListener(cancelButton->onPress, this, &ExportScreen::buttonPressed);
	view->add(cancelButton);
	
	
	formatSelectLabel = makeLabel("Format");
	formatSelect = new mui::SegmentedSelect<ExportFormat>();
	formatSelect->addSegment("h264", ExportFormat::H264);
	formatSelect->addSegment("png", ExportFormat::IMAGE_SEQUENCE_PNG);
	formatSelect->addSegment("tiff", ExportFormat::IMAGE_SEQUENCE_TIFF);
	formatSelect->setSelected(globals.exportFormat);
	view->add(formatSelect);
	
	filePickerLabel = makeLabel("Destination");
	filePicker = new mui::FilePicker(mui::FilePicker::Operation::SAVE);
	filePicker->setFormatter([](const std::filesystem::path & file){
		if(file=="") return string("-");
		else return ofFile(file,ofFile::Reference).getFileName();
	});
	filePicker->onFileChanged.add([&](){
		commit();
	});
	
	view->add(filePicker);
	
	
	auto makeTextArea = [&](int * globalProperty){
		mui::TextArea * text = new mui::TextArea(ofToString(*globalProperty),0,0,60,30);
		text->bg = ofColor(50);
		text->opaque = true;
		text->selectAllOnFocus = true;
		text->multiLine = false; 
		text->onCharacterAdded.add([](uint32_t & ch){
			return ch >= '0' && ch <= '9';
		});
		// yes, really, mui has no tab cycling yet... so for now.... :/
		text->onKeyPressed.add([this,text](ofKeyEventArgs & args){
			if(args.key == OF_KEY_TAB || args.key == OF_KEY_RETURN){
				// find the next ...
				auto children = findChildrenOfType<mui::TextArea>();
				if(children.size()>0){
					size_t idx = find(children.begin(),children.end(),text)-children.begin();
					if(ofGetKeyPressed(OF_KEY_SHIFT)) idx += children.size()-2;
					children[(idx+1)%children.size()]->requestKeyboardFocus();
					children[(idx+1)%children.size()]->selectAll();
				}
				return true;
			}
			else{
				return false;
			}
		});
		
		view->add(text);
		
		return text;
		
	};
	
	sizeLabel = makeLabel("Size");
	sizeXLabel = makeLabel("x");
	sizeXLabel->sizeToFitWidth(5);
	sizeAtLabel = makeLabel("@");
	sizeAtLabel->sizeToFitWidth(5);
	sizeFpsLabel = makeLabel("fps");
	sizeFpsLabel->sizeToFitWidth(5);
	
	
	widthText = makeTextArea(&globals.exportWidth);
	heightText = makeTextArea(&globals.exportHeight);
	fpsText = makeTextArea(&globals.exportFrameRate);
	fpsText->width = 40;
}

ExportScreen::~ExportScreen(){
	delete view;
	delete filePickerLabel;
	delete formatSelectLabel;
	delete sizeLabel;
	delete sizeXLabel;
	delete sizeAtLabel;
	delete sizeFpsLabel;
	delete widthText;
	delete heightText;
	delete fpsText;
	delete formatSelect;
	delete filePicker;
	delete exportButton;
	delete cancelButton;
	delete resetButton;
}


void ExportScreen::layout(){
	view->width = 500;
	
	mui::L(formatSelectLabel).posTL(10,10);
	mui::L(formatSelect).rightOf(formatSelectLabel,10).stretchToRightEdgeOfParent(10);

	mui::L(filePickerLabel).below(formatSelectLabel,10);
	mui::L(filePicker).rightOf(filePickerLabel,10).stretchToRightEdgeOfParent(10);
	
	mui::L(sizeLabel).below(filePickerLabel,10);
	mui::L({widthText,sizeXLabel,heightText,sizeAtLabel,fpsText,sizeFpsLabel}).columns(sizeLabel->getBounds().getTopRight()+ofVec3f(10,0));
	
	mui::L(exportButton).below(sizeLabel,20);
	mui::L(cancelButton).rightOf(exportButton,2);
	mui::L(resetButton).rightOf(cancelButton,2);
	
	view->height = view->getChildBounds().getBottom()+10;
	
	
	mui::L(view).spreadEvenlyHorizontally(0, width, mui::Center).spreadEvenlyVertically(0, height,mui::Middle);
}

void ExportScreen::beginExport(){
	globals.exportWidth = max(2,ofToInt(widthText->getText()));
	globals.exportHeight = max(2,ofToInt(heightText->getText()));
	globals.exportFrameRate = max(2,ofToInt(fpsText->getText()));
	globals.exportFormat = formatSelect->getSelectedValueOr(ExportFormat::H264);


	ofSendMessage("begin-export");
	visible = false;
}

void ExportScreen::buttonPressed(const void * sender, ofTouchEventArgs & args ){
	
	if(sender == exportButton ){
		ExportFileInfo info = getFileInfo();
		
		if(info.error != ""){
			//ofSystemAlertDialog(info.error);
			show_ok_dialog(info.error, [](){});
			return;
		}
		
		if((std::filesystem::path)info.file==""){
			show_ok_dialog("Please select a valid output directory/file first", [](){});
			return;
		}
		
		if(info.exists){
			show_yes_no_dialog("Destination already exists. Overwrite?", [&](bool yes){
				if(yes) beginExport();
			});
		}
		else{
			beginExport();
		}
		
	}
	else if(sender == cancelButton){
		visible = false;
	}
	else if(sender == resetButton){
		widthText->setTextAndNotify("1920");
		heightText->setTextAndNotify("1080");
		fpsText->setTextAndNotify("60");
	}
}

void ExportScreen::show(const ofFile & file){
	defaultName = file.getBaseName();
	filePicker->setDefaultSaveName(defaultName);
	commit();
	visible = true;
	handleLayout();
}

const std::filesystem::path ExportScreen::getFile(){
	return getFileInfo().file;
}

const ExportFileInfo ExportScreen::getFileInfo(){
	std::filesystem::path in_path(filePicker->getFile());
	ofFile in_file(in_path, ofFile::Reference);
	ExportFormat format = getFormat();

	if(in_path == ""){
		return ExportFileInfo{
			.file = std::move(in_file),
			.exists = false,
			.error = "Please select a valid file/directory",
			.sampleRate = 96000,
			.format = format
		};
	}
	

	bool is_dir;
	std::string ext;
	int sampleRate;
	
	
	switch(format){
		case ExportFormat::H264:
			is_dir = false;
			ext = "mp4";
			sampleRate = 96000;
			break;
		case ExportFormat::IMAGE_SEQUENCE_TIFF:
			is_dir = true;
			ext = "tiff";
			sampleRate = 96000;
			break;
		case ExportFormat::IMAGE_SEQUENCE_PNG:
			is_dir = true;
			ext = "png"; break;
			sampleRate = 96000;
	}
	
	ofFile file;
	if(!is_dir && in_file.getExtension() != ext){
		file = ofFile(in_file.getAbsolutePath() + "." + ext, ofFile::Reference);
	}
	else{
		file = in_file;
	}
	
	bool exists = file.exists();
	
	std::string error;
	if(exists && is_dir != in_file.isDirectory()){
		if(is_dir) error = "Expected directory, but got file";
		else error = "Expected file, but got directory";
	}
	
	return ExportFileInfo{
		.file = std::move(file),
		.exists = exists,
		.error = error,
		.sampleRate = sampleRate,
		.format = format
	};
}


ExportFormat ExportScreen::getFormat(){
	return formatSelect->getSelectedValueOr(ExportFormat::H264);
}

void ExportScreen::commit(){
	ExportFileInfo info = getFileInfo();
	
	filePicker->bg = info.exists && info.error == ""?
		ofColor( 255, 128, 128, 50 ) :
		ofColor( 128, 50 );

}
