#pragma once
#include "ofMain.h"

// this is basically https://github.com/companje/ofxExtras
// with lots things deleted. 
void ofxExit(string error );
bool ofxFileExists(string filename);
bool ofxStringEndsWith(string str, string key);
string ofxFormatString(string format, int number);
string ofxFormatString(string format, string s);
string ofxReplaceString(string input, string replace, string by);
void ofxSaveString(string filename, string str);
void ofxSaveStrings(string filename, vector<string> lines);
string ofxTrimStringRight(string str);
string ofxTrimStringLeft(string str);
string ofxTrimString(string str);
string ofxStringBeforeFirst(string str, string key);
string ofxStringAfterFirst(string str, string key);
string ofxStringAfterLast(string str, string key);
string ofxStringBeforeLast(string str, string key);
bool ofxContains(vector<string> keys, string key);
float ofxDist(float ax, float ay, float az, float bx, float by, float bz);
bool ofxColorMatch(ofColor a, ofColor b, int tolerance);
void ofxScale(float scale);
void ofxSetHexColor(int hexColor, int a); //alpha between 0..255
void ofxSetColor(ofColor c);
void ofxSetColorHSB(int h, int s, int b, int a);
bool ofxToBoolean(string str); //rick 3-1-2008
bool ofxToBoolean(float f);
int ofxToInteger(string str);
string ofxToString(char ch);
string ofxToString(unsigned char ch);
string ofxToString(string str);
string ofxToString(float f);
string ofxToString(bool value);
string ofxToString(int value);
string ofxToString(ofRectangle v);
ofColor ofxToColor(int hexColor);
ofColor ofxToColor(unsigned char r, unsigned char g, unsigned char b);
ofColor ofxToColor(ofVec4f v);
ofColor ofxToColor(ofVec3f v, int alpha);
string ofxToHexString(int value, int digits=6);
int ofxToInteger(ofColor c);
string ofxToString(ofQuaternion q);
ofQuaternion ofxToQuaternion(string str);
string ofxToHex(char c);
void ofxRotate(ofVec3f v);
void ofxRotate(ofQuaternion q);
void ofxRotate(float angle, ofVec3f v);
void ofxTranslate(ofVec3f v);
void ofxScale(ofVec3f v);
ofVec2f ofxToVec2f(string str);
ofVec3f ofxToVec3f(string str);
ofVec3f ofxToVec3f(float *a);
ofVec4f ofxToVec4f(string str);
ofRectangle ofxToRectangle(ofVec4f v);
ofRectangle ofxToRectangle(string str);
string ofxToString(ofVec2f v);
string ofxToString(ofVec3f v, int precision);
string ofxToString(ofVec4f v);
string ofxToString(ofMatrix4x4 m);
ofMatrix4x4 ofxToMatrix4x4(string s);
vector<float> ofxToFloatVector(string s, string delimiter);
