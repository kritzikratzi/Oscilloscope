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

void OsciMesh::addLines( float * left, float * right, int N ){
	addLine(last,{left[0],right[0]});
	last = {left[N-1],right[N-1]};
	
	for( int i = 1; i < N; i++ ){
		ofVec2f p0(left[i-1], right[i-1]);
		ofVec2f p1(left[i  ], right[i  ]);
		addLine(p0,p1);
	}
}

inline void OsciMesh::addLine(ofVec2f p0, ofVec2f p1){
	ofVec2f dir = p1 - p0;
	float z = dir.length();
	if (z > EPS) dir /= z;
	else dir = ofVec2f(1.0, 0.0);
	
	dir *= uSize;
	ofVec2f norm(-dir.y, dir.x);
	ofVec2f xy(-uSize, -uSize);
	
	mesh.addVertex(ofVec3f(p0-dir-norm));
	mesh.addColor(ofFloatColor(-uSize, -uSize, z));
	
	mesh.addVertex(ofVec3f(p0-dir+norm));
	mesh.addColor(ofFloatColor(-uSize, uSize, z));
	
	mesh.addVertex(ofVec3f(p1+dir-norm));
	mesh.addColor(ofFloatColor(z+uSize, -uSize, z));
	
	
	mesh.addVertex(ofVec3f(p0-dir+norm));
	mesh.addColor(ofFloatColor(-uSize, uSize, z));
	
	mesh.addVertex(ofVec3f(p1+dir-norm));
	mesh.addColor(ofFloatColor(z+uSize, -uSize, z));
	
	mesh.addVertex(ofVec3f(p1+dir+norm));
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