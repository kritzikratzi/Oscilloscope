//
//  SharedTex.cpp
//  Oscilloscope
//
//  Created by Hansi on 27/06/19.
//
//

#include "TexShare.h"
#include "ofMain.h"

#ifdef TARGET_OSX
#include "ofxSyphon.h"
class TexShareImpl{
public:
	ofxSyphonServer server;
	
	void setup(string name){
		server.setName(name);
	}
	
	void update(ofTexture &tex){
		server.publishTexture(&tex);
	}
};

#elif defined TARGET_WIN32
#include "ofxSpout.h"
class TexShareImpl{
public:
	ofxSpout::Sender spoutSender;
	
	void setup(string name){
		spoutSender.init(name);
	}
	
	void update(ofTexture &tex){
		spoutSender.send(tex);
	}
};

#else
class TexShareImpl{
public:
	void setup(string name) {}
	
	void update(ofTexture &tex) {}
};

#endif

TexShare::TexShare(){
	impl = make_unique<TexShareImpl>();
}

TexShare::~TexShare() = default;

void TexShare::setup(string name){
	impl->setup(name);
}

void TexShare::update(ofTexture &tex){
	impl->update(tex);
}
