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
	
	// adds a lot of lines.
	// left: x coordinates (ranged -1..1)
	// right: y coordinates (ranged -1..1)
	// zMod: brightness control (ranged 0..1), nullable
	// stride: stride for left and right (doesn't apply to bright!)
	void addLines( float * left, float * right, float * bright, int N, int stride);
	
	// adds a line from a to b (xy = -1..1), with the brightness bright (0..1)
	inline void addLine( ofVec2f a, ofVec2f b, const float bright );
	
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
