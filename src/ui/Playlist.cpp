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

	loopModeSelect = new mui::SegmentedSelect<LoopMode>(0,0,60*5,30);
	loopModeSelect->equallySizedButtons = true; 
	loopModeSelect->buttonSeparatorColor = ofColor(125, 50);
	loopModeSelect->buttonBgDefault = ofColor(128, 50);
	loopModeSelect->buttonBgSelected = ofColor(255);
	loopModeSelect->buttonFgDefault = ofColor(255);
	loopModeSelect->buttonFgSelected = ofColor(0); 
	loopModeSelect->buttonFontName = "";
//	tweak(loopModeSelect->addSegment(ofxFontAwesome4::sort_alpha_asc, LoopMode::ONCE), ofxFontAwesome4::_filename);
//	tweak(loopModeSelect->addSegment(ofxFontAwesome4::repeat, LoopMode::ALL), ofxFontAwesome4::_filename);
	tweak(loopModeSelect->addSegment("abcd", LoopMode::abcd), "");
	tweak(loopModeSelect->addSegment("abab", LoopMode::abab), "");
	tweak(loopModeSelect->addSegment("a", LoopMode::a), "");
	tweak(loopModeSelect->addSegment("aaaa", LoopMode::aaaa), "");
	tweak(loopModeSelect->addSegment("????", LoopMode::random), "");
	add(loopModeSelect);


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
	mui::L(loopModeSelect).rightOf(clearButton, 15);
	mui::L(shuffleToggle).rightOf(loopModeSelect, 15);

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
		for( auto file : dir.getFiles() ){
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
	switch (loopModeSelect->getSelectedValueOr(LoopMode::abcd)) {
	case LoopMode::abcd: {
		return getNextItem(itemId);
	}
	case LoopMode::abab: {
		auto res = getNextItem(itemId);
		if (res.first == 0) res = getNextItem(res.first);
		return res;
	}
	case LoopMode::a: {
		return{ 0,"" };
	}
	case LoopMode::aaaa: {
		return{ itemId,getItemPath(itemId) };
	}
	case LoopMode::random: {
		auto & children = scroller->view->children; 
		if (children.size() == 0) return{ 0,"" };
		
		auto idx = rand() % children.size(); 
		auto item = dynamic_cast<PlaylistItem*>(children[idx]);
		if (item) return{ item->itemId, filenames.find(item->itemId)->second };
		else return {0, ""};
	}
	}

	
	return {0,""};
}

string Playlist::getItemPath( size_t itemId ){
	auto it = filenames.find(itemId);
	return it == filenames.end()? "":it->second;
}

void Playlist::buttonPressed(const void * sender, ofTouchEventArgs & args) {
	if (sender == clearButton) {
		removeAllFiles();
	}
	else if (sender == addFileButton) {
		auto res = ofSystemLoadDialog("Add Folder", true);
		if (res.bSuccess) {
			addFile(ofFile(res.filePath, ofFile::ReadOnly));
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
	ofSendMessage("load-id:" + ofToString(itemId));
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
