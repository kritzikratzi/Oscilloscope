//
//  ShaderLoader.h
//  Oscilloscope
//
//  Created by Hansi on 21.08.15.
//
//

#ifndef Oscilloscope_ShaderLoader_h
#define Oscilloscope_ShaderLoader_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>
#include "ofMain.h"
#include "../globals.h"

class ShaderLoader{
	string fragFile;
	string vertFile;
	string geomFile;
	ofShader * shader;
	
	long fragTime;
	long vertTime;
	long geomTime;
	
	
public:
	
	ShaderLoader(){
		
	}
	
	void setup( ofShader * shader, string baseName ){
		this->shader = shader; 
		fragFile = ofxToReadonlyDataPath(baseName + ".frag");
		vertFile = ofxToReadonlyDataPath(baseName + ".vert");
		geomFile = ofxToReadonlyDataPath(baseName + ".geom");
		
		ofAddListener( ofEvents().update, this, &ShaderLoader::of_update, OF_EVENT_ORDER_BEFORE_APP );
		ofAddListener( ofEvents().draw, this, &ShaderLoader::of_draw, OF_EVENT_ORDER_AFTER_APP );
	}
	
	~ShaderLoader(){
		ofRemoveListener( ofEvents().update, this, &ShaderLoader::of_update, OF_EVENT_ORDER_BEFORE_APP );
		ofRemoveListener( ofEvents().draw, this, &ShaderLoader::of_draw, OF_EVENT_ORDER_AFTER_APP );
	}
	
	void of_update( ofEventArgs &args ){
		if( ofGetFrameNum() % 50 == 0 ){
			time_t fragTimeNow = mtime(fragFile);
			time_t vertTimeNow = mtime(vertFile);
			time_t geomTimeNow = mtime(geomFile);
			if( fragTimeNow != fragTime || vertTimeNow != vertTime || geomTimeNow != geomTime ){
				cout << "Reload ... " << endl;
				if( shader->isLoaded() ) shader->unload(); 
//				shader->setGeometryInputType(GL_LINES); // LINE_STRIP_ADJACENCY_EXT
//				shader->setGeometryOutputType(GL_TRIANGLE_STRIP); // take care, quads dont work on most cards, even with GL2!
//				shader->setGeometryOutputCount(4);
				shader->load(vertFile, fragFile/*, geomFile*/); // don't use geometry shaders because intelhd3000?
			}
			
			fragTime = fragTimeNow;
			vertTime = vertTimeNow;
			geomTime = geomTimeNow;
		}
	}
	
	
	void of_draw( ofEventArgs &args ){
		// TODO:
		// draw in case of compilation error!
	}
	
	
	
	
	
	// http://stackoverflow.com/a/10446821/347508
	static time_t mtime(string path) {
		struct stat attr;
		stat(path.c_str(), &attr);
		time_t res = attr.st_mtime;
		return res;
	}
};


#endif
