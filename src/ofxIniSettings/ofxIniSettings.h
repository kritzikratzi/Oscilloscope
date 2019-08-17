#ifndef OFX_INISETTINGS_H
#define OFX_INISETTINGS_H

#include <fstream>
#include <iostream>
#include <map>

#include "ofMain.h"

class ofxIniSettings {
public:

    ofxIniSettings() {}
    ofxIniSettings(string filename) { load(filename); }

    bool load(string filename, bool clearFirst=false, bool setAsOutputFile=true); // you can call multiple times with different files, incremental
    bool has(string key) { return keys[key]!=""; };
    void clear();
    string replaceVariables(string value);
    void print();

    map<string,string> keys;
    string outputFilename;

    //template<typename T> T operator[](const string& key)

    //getters
    int get(string key, int defaultValue);
    bool get(string key, bool defaultValue);
    float get(string key, float defaultValue);
    string get(string key, string defaultValue);
    string get(string key, const char* defaultValue);
    ofVec2f get(string key, ofVec2f defaultValue);
    ofVec3f get(string key, ofVec3f defaultValue);
    ofVec4f get(string key, ofVec4f defaultValue);
    ofRectangle get(string key, ofRectangle defaultValue);
    ofQuaternion get(string key, ofQuaternion defaultValue);
    ofMatrix4x4 get(string key, ofMatrix4x4 defaultValue);

    //WORKS: string operator[](string key) { return get(key,""); }  but not with multiple overloading

    int getInt(string key) { return get(key,0); }
    string getString(string key) { return get(key,""); }
    float getFloat(string key) { return get(key,0.0f); }
    ofColor getColor(string key) { return ofColor::fromHex(getInt(key)); }
    bool getBool(string key) { return get(key,false); }

    //int operator[](string key) { return get(key,0); }    //cannot overload previous one
    //template<typename T> operator [](const string& x) { return  };

//    int get(string key) { return get(key,0); }
//    bool get(string key) { return get(key,false); }
//    float get(string key) { return get(key,0.0f); }
//    string get(string key) { return get(key,""); }
//    ofVec2f get(string key) { return get(key,ofVec2f());
//    ofVec3f get(string key) { return get(key,ofVec3f());
//    ofVec4f get(string key) { return get(key,ofVec4f());

    //template<typename T> operator [](const string& x) { return };
    //template<typename T> T operator[](const string& key) { return get(key,T()); }
    // string operator[](const string& key) { return get(key,string()); }
    //ofTrueTypeFont &operator[](const string& fontnamesize);

    //setters
    void setString(string id, string value);

    template<typename T> void set(string id, string key, T value) { set(id=="" ? key : (id+"."+key), value); } //returntype void
    template<typename T> void set(string key, T value) { setString(key, ofxToString(value)); } //returntype void

	
	
	// getting rid of inter-dependencies to ofxExtras. here's the stuff thats needed:
	static void ofxExit(string error );
	static bool ofxFileExists(string filename) ;
	bool ofxStringEndsWith(string str, string key) ;
	static string ofxFormatString(string format, int number) ;
	static string ofxFormatString(string format, string s) ;
	static string ofxReplaceString(string input, string replace, string by) ;
	static void ofxSaveString(string filename, string str) ;
	static void ofxSaveStrings(string filename, vector<string> lines) ;
	static string ofxTrimStringRight(string str) ;
	static string ofxTrimStringLeft(string str) ;
	static string ofxTrimString(string str) ;
	static string ofxStringBeforeFirst(string str, string key) ;
	static string ofxStringAfterFirst(string str, string key) ;
	static string ofxStringAfterLast(string str, string key) ;
	static string ofxStringBeforeLast(string str, string key) ;
	static bool ofxContains(vector<string> keys, string key) ;
	static float ofxDist(float ax, float ay, float az, float bx, float by, float bz) ;
	static bool ofxColorMatch(ofColor a, ofColor b, int tolerance) ;
	static void ofxScale(float scale) ;
	static void ofxSetHexColor(int hexColor, int a);
	static void ofxSetColor(ofColor c) ;
	static void ofxSetColorHSB(int h, int s, int b, int a) ;
	static bool ofxToBoolean(string str);
	static bool ofxToBoolean(float f) ;
	static int ofxToInteger(string str) ;
	static string ofxToString(char ch);
	static string ofxToString(unsigned char ch);
	static string ofxToString(string str);
	static string ofxToString(float f) ;
	static string ofxToString(bool value);
	static string ofxToString(int value) ;
	static string ofxToString(ofRectangle v) ;
	static ofColor ofxToColor(int hexColor) ;
	static ofColor ofxToColor(unsigned char r, unsigned char g, unsigned char b) ;
	static ofColor ofxToColor(ofVec4f v) ;
	static ofColor ofxToColor(ofVec3f v, int alpha) ;
	static string ofxToHexString(int value, int digits=6) ;
	static int ofxToInteger(ofColor c) ;
	static string ofxToString(ofQuaternion q) ;
	static ofQuaternion ofxToQuaternion(string str) ;
	static string ofxToHex(char c) ;
	static vector<string> ofxToStringVector(string value) ;
	static ofVec2f ofxToVec2f(string str) ;
	static ofVec3f ofxToVec3f(string str) ;
	static ofVec3f ofxToVec3f(float *a) ;
	static ofVec4f ofxToVec4f(string str) ;
	static ofRectangle ofxToRectangle(ofVec4f v) ;
	static ofRectangle ofxToRectangle(string str) ;
	static string ofxToString(ofVec2f v) ;
	static string ofxToString(ofVec3f v, int precision) ;
	static string ofxToString(ofVec4f v) ;
	static string ofxToString(ofMatrix4x4 m) ;
	static ofMatrix4x4 ofxToMatrix4x4(string s) ;
	static vector<float> ofxToFloatVector(string s, string delimiter) ;
};

#endif

