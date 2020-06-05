//
//  Playlist.hpp
//  Oscilloscope
//
//  Created by Hansi on 05.08.17.
//
//

#ifndef Playlist_hpp
#define Playlist_hpp

#include "ofxMightyUI.h"

class PlaylistItem;
class FaButton; 
class FaToggleButton; 

struct PlaylistItemRef{
	PlaylistItemRef(){}
	PlaylistItemRef(size_t id, string filename) : id(id), filename(filename){}
	size_t id{0};
	string filename{""};
};

class Playlist : public mui::Container{
public:
	Playlist();
	~Playlist();
	
	void draw() override;
	void update() override;
	void layout() override;
	bool fileDragged(ofDragInfo & args) override; 
	bool keyPressed(ofKeyEventArgs & args) override;
	
	PlaylistItemRef addFile(ofFile file, double duration = -2 );
	void removeAllFiles();
	
	void save(ostream & out);
	void load(istream & in);
	
	
	// finds the next item in the playlist
	// if id=0, the first item is returned.
	// returns: {0,""} if there is no further item, otherwise returns the thing you want.
	PlaylistItemRef getNextItemInPlaylist(size_t id);
	string getItemPath(size_t id); 

	enum class LoopMode { all_once, all_repeat, one_repeat };

private:
	PlaylistItemRef getNextItem(size_t id);

	void setLoopMode(LoopMode mode); 
	void buttonPressed(const void * sender, ofTouchEventArgs & args); 

	mui::ScrollPane * scroller;
	bool checkNewFiles = false;
	size_t nextItemId = 1;
	map<size_t, string> filenames; 

	FaButton * loopModeButton; 
	FaToggleButton * shuffleToggle; 
	FaToggleButton * onlyOneButton; 

	FaButton * clearButton;
	FaButton * addFileButton;
};


class PlaylistItem : public mui::Label{
public:
	PlaylistItem(size_t itemId, ofFile file, double duration = -2);
	
	void touchDown(ofTouchEventArgs & args) override;
	void touchUp(ofTouchEventArgs & args) override;
	void touchUpOutside(ofTouchEventArgs & args) override;
	void mouseEnter(ofMouseEventArgs &args) override;
	void mouseExit(ofMouseEventArgs &args) override;
	void drawBackground() override;
	
	
	void updateDuration(bool & alreadyScanned, bool & shouldRemove );
	
	size_t itemId; 
	bool selected = false;
	bool hovering = false;
	ofFile file;
	double duration = -2; // -2 = didn't look, -1 = error, > 0 is okay
};

#endif /* Playlist_hpp */
