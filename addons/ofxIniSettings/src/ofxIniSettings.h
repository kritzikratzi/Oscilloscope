#ifndef OFX_INISETTINGS_H
#define OFX_INISETTINGS_H

#include <fstream>
#include <iostream>
#include <map>

#include "ofMain.h"
#include "ofxExtras.h"

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

};

#endif

