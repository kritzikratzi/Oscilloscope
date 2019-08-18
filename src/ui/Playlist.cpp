//
//  Playlist.cpp
//  Oscilloscope
//
//  Created by Hansi on 05.08.17.
//
//

#include "Playlist.h"
#include "MuiL.h"
#include "ofxAvUtils.h"
#include "../globals.h"
#include "ofxFontAwesome.h"
#include "FaButton.h"
#include "FaToggleButton.h"
#include "FMenu.h"
#include "ofxNative.h"

namespace natural_sort{
	int compare(const string & a, const string & b);
}

Playlist::Playlist() : mui::Container(0,0,300,500){
	name = "playlist";
	bg = ofColor(125, 50);
	opaque = true;

	header = new mui::Label("Playlist");
	header->fontSize = 20;
	header->height = 30;
	header->commit(); 
	add(header);
	
	scroller = new mui::ScrollPane();
	scroller->canScrollX = false;
	scroller->canScrollY = true;
	scroller->view->layoutHandler = new mui::RowLayout(1);
	add(scroller);

	
	auto tweak = [](mui::SegmentedButton<LoopMode> * btn, const string & filename ) {
		btn->label->fontName = filename;
		//btn->label->verticalAlignUseBaseline = false;
		btn->label->fontSize = 8; 
		btn->label->commit(); 
	};

	loopModeButton = new FaButton(ofxFontAwesome::sort_alpha_asc,0,0, 30, 30);
	ofAddListener(loopModeButton->onPress, this, &Playlist::buttonPressed);
	add(loopModeButton);
	setLoopMode(LoopMode::all_repeat);


	shuffleToggle = new FaToggleButton(ofxFontAwesome::random, ofxFontAwesome::random, 0, 0, 30, 30);
	ofAddListener(shuffleToggle->onPress, this, &Playlist::buttonPressed);
	add(shuffleToggle);

	clearButton = new FaButton(ofxFontAwesome::trash,0,0,30,30);
	clearButton->label->inset.top = -4;
	ofAddListener(clearButton->onPress, this, &Playlist::buttonPressed);
	add(clearButton); 

	addFileButton = new FaButton(ofxFontAwesome::folder_open,0,0,30,30);
	addFileButton->label->inset.top = -2;
	addFileButton->label->inset.left = 2;
	ofAddListener(addFileButton->onPress, this, &Playlist::buttonPressed);
	add(addFileButton); 

}

Playlist::~Playlist(){
	for( mui::Container * c : scroller->view->children ){
		delete c;
	}
	scroller->view->children.clear();
	delete scroller->view->layoutHandler;
	scroller->view->layoutHandler = nullptr;
	delete scroller;
	delete header;
}

void Playlist::update(){
	if(checkNewFiles){
		bool foundStuff = false;
		
		for( mui::Container * el : scroller->view->children ){
			PlaylistItem * item = dynamic_cast<PlaylistItem*>(el);
			if(item){
				bool alreadyScanned = false, shouldRemove = false;
				item->updateDuration(alreadyScanned, shouldRemove );
				if(alreadyScanned) continue;
				if(shouldRemove){
					el->remove();
					delete el;
					scroller->commit();
					foundStuff = true;
					break;
				}
			}
		}
		
		if(!foundStuff){
			checkNewFiles = false;
		}
	}
}

void Playlist::draw(){
	ofDrawLine(width,0,width,height);
}

void Playlist::layout(){
	mui::L(header).posTL(0,-header->height).stretchToRightEdgeOfParent();
	mui::L(addFileButton).below(header, 1);
	mui::L(clearButton).rightOf(addFileButton, 1);
	mui::L(loopModeButton).rightOf(clearButton, 15);
	mui::L(shuffleToggle).rightOf(loopModeButton, 1);

	mui::L(scroller).below(addFileButton).stretchToRightEdgeOfParent().stretchToBottomEdgeOfParent(30);
	

	
}

bool Playlist::fileDragged( ofDragInfo & args ){
	for( auto & file : args.files ){
		addFile(ofFile(file, ofFile::Reference));
	}
	
	scroller->handleLayout();
	scroller->commit();
	
	return true;
}


bool Playlist::keyPressed(ofKeyEventArgs & args){
	if(args.key == OF_KEY_RETURN){
		cout << "play this now" << endl;
		return true;
	}
	else if(args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE){
		cout << "do this then" << endl;
		return true;
	}
	
	checkNewFiles = true;
	
	// we don't care
	return false;
}


void Playlist::addFile( ofFile file, double duration ){
	checkNewFiles = true;
	
	if(file.isDirectory()){
		// step in!
		ofDirectory dir(file.getAbsolutePath());
		auto files = dir.getFiles();
		stable_sort(files.begin(), files.end(), [](const ofFile & a, const ofFile & b){
			return natural_sort::compare(a.getBaseName(), b.getBaseName())<0;
		});
		
		for( auto file : files ){
			addFile(file); 
		}
	}
	else{
		static set<string> allowed = {
			"wav", "mp2", "mp3", "mp4", "h264", "xvid", "flac", "aac", "aiff", "aif", "mts","mov",
			"mkv", "3gp", "nsf", "avi", "3ga", "aa3", "ab", "abm", "ac3", "acm", "acp", "act", "adt",
			"afc", "aifc", "ang", "aria", "at3", "au", "aup", "brstm", "caf", "caff", "cda", "cdda", "cdlx",
			"cel", "cfa", "ckb", "ckf", "cpt", "csh", "dcm", "dct", "dff", "dig", "f64", "f32", "g721","g723","g726",
			"gsf", "gsm", "hma", "m4a", "m4b", "m4p", "mka", "mo3", "mpa", "mpga", "mpu", "mus", "rbs",
			"wave", "u", "tta", "swa", "nd", "raw", "flv", "ogg", "m4v", "gsm", "ogv", "swf"
		};
		
		static set<string> forbidden = {
			"pdf", "ai", "psd", "zip", "txt", "jpg", "jpeg", "gif", "png", "cdr", "svg", "rtf",
			"xls", "ods", "blend", "blend1", "tiff", "os", "stl", "html", "htm", "js", "cpp", "h",
			"c", "java", "go", "cs", "mtl", "obj", "sldprt", "sldasm", "eps", "gz", "out", "ttf",
			"docx", "xlsx", "mid", "torrent",
			"fingerbaby", "json", "sql", "ino", "eml", "p12", "csv", "hqx", "css", "md",
			"ml", "zi", "zo", "mli", "zix", "a", "1", "iso", "2", "3", "pcvd", "cmake",
			"bin", "make", "o", "check_cache", "log", "cxx", "marks", "dylib", "dat",
			"dia", "pbxproj", "xcworkspacedata", "xcuserstate", "xcsettings", "xcscheme",
			"plist", "hmap", "bat", "xml", "eot", "woff", "woff2", "icns", "frag", "vert",
			"dmg", "otf", "asd", "ps", "tif", "automap", "7z", "inc", "orig", "rej",
			"pptx", "part", "php", "certsigningrequest", "sch", "mak", "mms", "rsp", "opt",
			"asm", "387", "688", "87", "amd64", "tgz", "nb", "modulemap", "crt", "pem",
			"entitlements", "strings", "nib", "sdef", "pde", "mvc", "cfg", "jar", "ver",
			"ctdc", "ast", "vst3", "aup", "vcproj", "sln", "bz2", "qtz", "ico", "ics",
			"class", "dex", "apk", "ap_", "properties", "db", "db-shm", "db-wal", "data",
			"maps", "pcap", "in", "m", "yml", "py", "helpindex", "m4", "lo", "0", "pc",
			"inf", "mmp", "cygwin", "guess", "status", "sub", "ac", "am", "rc", "man",
			"amiga", "cross", "autoconf", "cords", "darwin", "dgux386", "dj", "environment",
			"ews4800", "hp", "linux", "mac", "macros", "os2", "rs6000", "sgi", "solaris2",
			"symbian", "uts", "win32", "win64", "cc", "s", "template", "msft", "la",
			"direct", "quick", "glsl", "exe", "vcxproj", "filters", "user", "xz", "dist",
			"ini", "lock", "cnf", "mountainducklicense", "doc", "xlw", "rar", "srt",
			"nfo", "pub", "m3u", "ild", "msi", "m3u8", "midi", "cer", "zxp", "3ds",
			"repository", "bmp", "base", "dsp", "dsw", "suo", "mxf", "hh", "simd",
			"ivml", "cpio", "hpp", "rst", "sphinx", "abilist", "exp",
			"includecache", "internal", "sh", "linkfilelist", "d", "ipp", "llvm", "pls",
			"amxd", "plugins", "pyc", "textile", "xcbkptlist", "patchfile", "mm", "mobileprovision",
			"pkg", "pf", "bfc", "src", "dir", "jfc", "access", "ja", "certs", "policy", "security",
			"libraries", "cyberduckprofile", "pd", "mc", "cmd", "amad", "aux", "tex", "geom",
			"dll", "provisionprofile", "mk", "xccheckout", "xcconfig", "cras", "pcapng", "diff",
			"rsrc", "po", "tcl", "pot", "osx", "awk", "dependencies", "gnu", "mingw", "msvc",
			"epub", "jsx", "lproj", "doxy", "hxx", "dev", "def", "dox", "unix", "mcp", "soundy",
			"odt", "patch", "url", "dependency", "scriptsuite", "scriptterminology", "rss", "ppt",
			"dropzone", "selfcontrol", "nef", "lua", "als", "rf", "s3m", "fxml", "mf", "scpt",
			"disabled", "xib", "sketch", "sst", "command", "mode1v3", "pbxuser", "pch", "dmgpkg",
			"xlsm", "pak", "info", "iml", "markdown", "3ctype", "old", "conf", "map",
			"deps", "targ", "opts", "gyp", "makefile", "gypi", "node", "dtd", "coffee", "scss",
			"resjson", "asar", "fcp", "vstpreset", "aep", "idx3-ubyte", "idx1-ubyte", "puml",
			"license", "cl", "proto", "mtdf", "tour", "sketchyscope", "scad", "settings", "elua",
			"sample", "dxf", "bom", "tag", "md5", "dot", "uidesc", "r", "pch++", "cw9prj",
			"plc", "cbp", "tar", "xap", "developerprofile",
		};
		
		string ext = ofToLower(file.getExtension());
		transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		
		if(duration >= 0 ){
			filenames[nextItemId] = file.getAbsolutePath();
			PlaylistItem * item = new PlaylistItem(nextItemId++, file, duration);
			scroller->view->add(item);
		}
		else if(forbidden.find(ext) == forbidden.end()){
			filenames[nextItemId] = file.getAbsolutePath();
			PlaylistItem * item = new PlaylistItem(nextItemId++, file);
			scroller->view->add(item);
		}
	}
}

void Playlist::removeAllFiles(){
	for( auto el : scroller->view->children ){
		PlaylistItem * item = dynamic_cast<PlaylistItem*>(el);
		if(item){
			MUI_ROOT->safeRemoveAndDelete(item);
		}
	}
}

void Playlist::save(ostream & out){
	for( auto el : scroller->view->children ){
		PlaylistItem * item = dynamic_cast<PlaylistItem*>(el);
		if(item && item->duration > 0 ){
			out << item->duration << " " << item->file.getAbsolutePath() << endl;
		}
	}
}

void Playlist::load(istream & in){
	double duration;
	string filename;
	while(in.good()){
		in>>duration;
		if(getline(in>>ws, filename).good()){
			addFile(ofFile(filename, ofFile::Reference), duration);
		}
	}
	
	scroller->commit();
}

pair<size_t, string> Playlist::getNextItem(size_t itemId) {
	bool takeNext = false; 

	for (auto el : scroller->view->children) {
		PlaylistItem * item = dynamic_cast<PlaylistItem*>(el);
		if (item) {
			if (itemId == 0) {
				return{ item->itemId, item->file.getAbsolutePath() };
			}

			if (item->itemId == itemId) {
				takeNext = true;
			}
			else if (takeNext) {
				return{ item->itemId, item->file.getAbsolutePath() };
			}
		}
	}

	return{ 0,"" };
}

pair<size_t, string> Playlist::getNextItemInPlaylist( size_t itemId ){
	LoopMode * btnMode = loopModeButton->getProperty<LoopMode>("loop_mode");
	LoopMode mode = btnMode ? *btnMode : LoopMode::all_repeat; 


	if (shuffleToggle->selected && mode != LoopMode::one_repeat) {
		auto & children = scroller->view->children;
		if (children.size() == 0) return{ 0,"" };

		auto idx = rand() % children.size();
		auto item = dynamic_cast<PlaylistItem*>(children[idx]);
		if (item) return{ item->itemId, filenames.find(item->itemId)->second };
		else return{ 0, "" };
	}

	switch (mode) {
	case LoopMode::all_once: {
		return getNextItem(itemId);
	}
	case LoopMode::all_repeat: {
		auto res = getNextItem(itemId);
		if (res.first == 0) res = getNextItem(res.first);
		return res;
	}
	case LoopMode::one_repeat: {
		return{ itemId,getItemPath(itemId) };
	}
	}

	
	return {0,""};
}

string Playlist::getItemPath( size_t itemId ){
	auto it = filenames.find(itemId);
	return it == filenames.end()? "":it->second;
}

void Playlist::setLoopMode(LoopMode mode) {
	loopModeButton->setProperty("loop_mode", mode);

	switch (mode) {
	case LoopMode::all_once:
		loopModeButton->label->fontName = ofxFontAwesome4::_filename;
		loopModeButton->label->setText(ofxFontAwesome4::sort_alpha_asc);
		break;
	case LoopMode::all_repeat:
		loopModeButton->label->fontName = ofxFontAwesome4::_filename;
		loopModeButton->label->setText(ofxFontAwesome4::repeat);
		break;
	case LoopMode::one_repeat:
		loopModeButton->label->fontName = "";
		loopModeButton->label->setText("1");
		break;
	}
}

void Playlist::buttonPressed(const void * sender, ofTouchEventArgs & args) {
	if (sender == clearButton) {
		removeAllFiles();
	}
	else if (sender == addFileButton) {
		auto res = ofSystemLoadDialog("Add Folder", true);
		if (res.bSuccess) {
			addFile(ofFile(res.filePath, ofFile::ReadOnly));
			handleLayout(); 
		}
	}
	else if (sender == loopModeButton) {
		LoopMode * btnMode = loopModeButton->getProperty<LoopMode>("loop_mode");
		LoopMode mode = btnMode ? *btnMode : LoopMode::all_repeat;

		switch (mode) {
		case LoopMode::all_once: setLoopMode(LoopMode::all_repeat); break;
		case LoopMode::all_repeat: setLoopMode(LoopMode::one_repeat); break;
		case LoopMode::one_repeat: setLoopMode(LoopMode::all_once); break;
		}
	}
}








PlaylistItem::PlaylistItem(size_t itemId, ofFile file, double duration) : mui::Label(file.getFileName()), itemId(itemId), file(file){
	opaque = true;
	ignoreEvents = false;
	fontSize = 10;
	inset.left = 25;
	
	if(duration >= 0){
		this->duration = duration;
	}
}

void PlaylistItem::updateDuration(bool & alreadyScanned, bool & shouldRemove ){
	if(duration==-2){
		duration = ofxAvUtils::duration_audio(file.getAbsolutePath());
		alreadyScanned = false;
		shouldRemove = duration <= 0;
	}
	else{
		alreadyScanned = true;
		shouldRemove = false;
	}
}


void PlaylistItem::touchDown(ofTouchEventArgs & args){
	if (muiIsContextClick()) {
		auto menu = new FMenu<string>(0, 0, 300, 0);
		menu->opaque = true;
		menu->onAfterRemove.add([](mui::Container * menu, mui::Container * parent) {
			MUI_ROOT->safeDelete(menu);
		});
		
		string path = file.getAbsolutePath(); 
		menu->addOption("Show file", "show-file", [menu,path]() {
			ofxNative::showFile(path); 
			MUI_ROOT->removePopup(menu); 
		}); 

		MUI_ROOT->showPopupMenu(menu, this, args.x, args.y, mui::Left, mui::Bottom);
	}
	else {
		ofSendMessage("load-id:" + ofToString(itemId));
	}
}

void PlaylistItem::touchUp(ofTouchEventArgs & args){
	
}

void PlaylistItem::touchUpOutside(ofTouchEventArgs & args){
	
}


void PlaylistItem::drawBackground(){
	ofColor bg(0,0);
	
	if(selected){
		bg = hovering?ofColor(50,50,100):ofColor(25,25,50);
	}
	else if(hovering){
		bg = ofColor(75);
	}
	
	if(duration == -1){
		bg.r *= 3;
	}
	
	if(bg.a>0){
		ofSetColor(bg);
		ofDrawRectangle(0,0,width,height);
	}
	
	if(globals.currentlyPlayingItem == itemId){
		ofSetColor(0,255,0);
		ofDrawTriangle({1,1}, {4,height/2}, {1,height-1});
	}
	
	ofSetColor(255);
}

void PlaylistItem::mouseEnter(ofMouseEventArgs &args){
	hovering = true;
}

void PlaylistItem::mouseExit(ofMouseEventArgs &args){
	hovering = false;
}





// NATURAL SORT COMPARATOR, ported to c++
/*
 NaturalOrderComparator.java -- Perform 'natural order' comparisons of strings in Java.
 Copyright (C) 2003 by Pierre-Luc Paour <natorder@paour.com>
 
 Based on the C version by Martin Pool, of which this is more or less a straight conversion.
 Copyright (C) 2000 by Martin Pool <mbp@humbug.org.au>
 
 This software is provided 'as-is', without any express or implied
 warranty.	In no event will the authors be held liable for any damages
 arising from the use of this software.
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 */

namespace natural_sort{
	char charAt(const string & s, int i){
		return i >= s.size()?0:s[i];
	}
	
	bool isDigit(unsigned char ch){
		return ch >= '0' && ch <= '9';
	}
	
	bool isSpaceChar(unsigned char ch){
		return ch == ' ';
	}

	int compareRight(const string & a, const string & b){
		int bias = 0;
		int ia = 0;
		int ib = 0;
		
		// The longest run of digits wins. That aside, the greatest
		// value wins, but we can't know that it will until we've scanned
		// both numbers to know that they have the same magnitude, so we
		// remember it in BIAS.
		for (;; ia++, ib++){
			unsigned char ca = charAt(a, ia);
			unsigned char cb = charAt(b, ib);
			
			if (!isDigit(ca) && !isDigit(cb)){
				return bias;
			}
			else if (!isDigit(ca)){
				return -1;
			}
			else if (!isDigit(cb)){
				return +1;
			}
			else if (ca < cb){
				if (bias == 0){
					bias = -1;
				}
			}
			else if (ca > cb){
				if (bias == 0)
					bias = +1;
			}
			else if (ca == 0 && cb == 0){
				return bias;
			}
		}
	}

	int compare(const string & a, const string & b)
	{
		int ia = 0, ib = 0;
		int nza = 0, nzb = 0;
		unsigned char ca, cb;
		int result;
		
		while (true){
			// only count the number of zeroes leading the last number compared
			nza = nzb = 0;
			
			ca = charAt(a, ia);
			cb = charAt(b, ib);
			
			// skip over leading spaces or zeros
			while (isSpaceChar(ca) || ca == '0'){
				if (ca == '0'){
					nza++;
				}
				else{
					// only count consecutive zeroes
					nza = 0;
				}
				
				ca = charAt(a, ++ia);
			}
			
			while (isSpaceChar(cb) || cb == '0'){
				if (cb == '0'){
					nzb++;
				}
				else{
					// only count consecutive zeroes
					nzb = 0;
				}
				
				cb = charAt(b, ++ib);
			}
			
			// process run of digits
			if (isDigit(ca) && isDigit(cb)){
				if ((result = compareRight(a.substr(ia), b.substr(ib))) != 0)
				{
					return result;
				}
			}
			
			if (ca == 0 && cb == 0){
				// The strings compare the same. Perhaps the caller
				// will want to call strcmp to break the tie.
				return nza - nzb;
			}
			
			if (ca < cb){
				return -1;
			}
			else if (ca > cb){
				return +1;
			}
			
			++ia;
			++ib;
		}
	}

}