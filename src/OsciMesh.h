//
//  OsciMesh.hpp
//  Oscilloscope
//
//  Created by Hansi on 12.01.17.
//
//

#ifndef OsciMesh_hpp
#define OsciMesh_hpp

#include "ofMain.h"
#include "util/ShaderLoader.h"

class OsciMesh{
public:
	OsciMesh();
	
	static void init();
	
	void addLines( float * left, float * right, int N, int stride);
	inline void addLine( ofVec2f a, ofVec2f b);
	
	void draw(ofMatrix4x4 & viewMatrix);
	void clear(); 
	
	// shader parameters
	float uSize;
	ofVec3f uRgb;
	float uIntensity;
	float uIntensityBase;
	float uHue;
	
	// it's a mesh. can you believe it?
	ofMesh mesh;
	
private:
	ShaderLoader shaderLoader;
	ofShader shader;
	ofVec2f last;
};

#endif /* OsciMesh_hpp */
