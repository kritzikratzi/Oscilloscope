//
//  OsciMesh.cpp
//  Oscilloscope
//
//  Created by Hansi on 12.01.17.
//
//

#define EPS 1E-6

#include "OsciMesh.h"

void OsciMesh::init(){
}

OsciMesh::OsciMesh(){
	shaderLoader.setup(&shader, "shaders/osci");
}

void OsciMesh::addLines( float * left, float * right, float * bright, int N, int stride ){
	// no work? go home watch tv or something
	if(N==0) return;
	
	addLine(last,{left[0],right[0]}, bright==nullptr?1:bright[0]);
	last = {left[N-stride],right[N-stride]};
	
	if(bright==nullptr){
		for( int i = stride; i < N; i+= stride ){
			ofVec2f p0(left[i-stride], right[i-stride]);
			ofVec2f p1(left[i  ], right[i  ]);
			addLine(p0,p1, 1);
		}
	}
	else{
		for( int i = stride; i < N; i+= stride ){
			ofVec2f p0(left[i-stride], right[i-stride]);
			ofVec2f p1(left[i  ], right[i  ]);
			addLine(p0,p1, bright[i]);
		}
	}
}

inline void OsciMesh::addLine(ofVec2f p0, ofVec2f p1, const float bright){
	ofVec2f dir = p1 - p0;
	float z = dir.length();
	if (z > EPS) dir /= z;
	else dir = ofVec2f(1.0, 0.0);
	
	dir *= uSize;
	ofVec2f norm(-dir.y, dir.x);
	ofVec2f xy(-uSize, -uSize);
	
	//make vec3 from vec2+float
	const auto vec3 = [bright](const ofVec2f & xy, const float & z){ return ofVec3f(xy.x,xy.y,bright); };

	mesh.addVertex(vec3(p0-dir-norm, bright));
	mesh.addColor(ofFloatColor(-uSize, -uSize, z));
	
	mesh.addVertex(vec3(p0-dir+norm, bright));
	mesh.addColor(ofFloatColor(-uSize, uSize, z));
	
	mesh.addVertex(vec3(p1+dir-norm, bright));
	mesh.addColor(ofFloatColor(z+uSize, -uSize, z));
	
	
	mesh.addVertex(vec3(p0-dir+norm, bright));
	mesh.addColor(ofFloatColor(-uSize, uSize, z));
	
	mesh.addVertex(vec3(p1+dir-norm, bright));
	mesh.addColor(ofFloatColor(z+uSize, -uSize, z));
	
	mesh.addVertex(vec3(p1+dir+norm, bright));
	mesh.addColor(ofFloatColor(z+uSize, +uSize, z));
};


void OsciMesh::draw(ofMatrix4x4 & viewMatrix){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	shader.begin();
	shader.setUniform3f("uRgb", uRgb);
	shader.setUniform1f("uSize", uSize);
	shader.setUniform1f("uIntensity", uIntensity);
	shader.setUniform1f("uIntensityBase", uIntensityBase);
	shader.setUniformMatrix4f("uMatrix", viewMatrix);
	shader.setUniform1f("uHue", uHue );
	ofSetColor(255);
	mesh.draw();
	shader.end();
	ofEnableAlphaBlending();
}


void OsciMesh::clear(){
	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	mesh.enableColors();
}