#include "ofxIniSettings.h"

#include "ofxExtras.h" //hier ipv in header scheelt compileer tijd wanneer ofxExtras veranderd

bool ofxIniSettings::load(string filename, bool clearFirst, bool setAsOutputFile) {
    if (setAsOutputFile) outputFilename = filename;
    filename = ofToDataPath(filename);
    ofLogVerbose() << "ini.loadSettings(\"" << filename << "\"";
    if (filename=="") { ofLog(OF_LOG_ERROR, "ofxIniSettings::no filename"); return false; }
    if (!ofxFileExists(filename)) { ofLog(OF_LOG_ERROR, "file not found: %s",filename.c_str()); return false; }
    string cmd,section,key,value,id;
    ifstream f(filename.c_str(),ios::in);
    if (clearFirst) clear();

    while (getline(f,cmd)) {
        cmd = ofxTrimStringRight(cmd); //remove \n \r \t space etc
        switch (cmd[0]) {
            case 0: break;   //empty first line?
            case '#': break; //comment
            case ';': break; //comment
            case '[': section = cmd.substr(1,cmd.length()-2); break; //section, remove brackets
            default:
                int pos = cmd.find("=", 0);
                if (pos==string::npos) break;
                key = cmd.substr(0, pos);
                value = cmd.substr(pos+1);
                id = section!="" ? (section + "." + key) : key;

                value = replaceVariables(value);

                //cout << "ini: " << id << " = " << value << endl;

                keys[id] = value;

        }
    }
    f.close();

    return true;
}

string ofxIniSettings::replaceVariables(string value) {
  //this function replaces all $(...) style variables with previously loaded values of the key names
  for (map<string,string>::iterator it=keys.begin(); it!=keys.end(); it++) {
      ofStringReplace(value,"$("+it->first+")",it->second);
  }
  return value;
}

void ofxIniSettings::clear() {
    keys.clear();
}

string   ofxIniSettings::get(string key, string   defaultValue) { return has(key) ? keys[key] : defaultValue; }
int      ofxIniSettings::get(string key, int      defaultValue) { return has(key) ? ofxToInteger(keys[key]) : defaultValue; }
float    ofxIniSettings::get(string key, float    defaultValue) { return has(key) ? ofToFloat(keys[key]) : defaultValue; }
bool     ofxIniSettings::get(string key, bool     defaultValue) { return has(key) ? ofxToBoolean(keys[key]) : defaultValue; }
string   ofxIniSettings::get(string key, const char*    defaultValue) { return has(key) ? keys[key] : defaultValue; }
ofVec2f  ofxIniSettings::get(string key, ofVec2f defaultValue) { return has(key) ? ofxToVec2f(keys[key]) : defaultValue; }
ofVec3f  ofxIniSettings::get(string key, ofVec3f defaultValue) { return has(key) ? ofxToVec3f(keys[key]) : defaultValue; }
ofVec4f  ofxIniSettings::get(string key, ofVec4f defaultValue) { return has(key) ? ofxToVec4f(keys[key]) : defaultValue; }
ofRectangle ofxIniSettings::get(string key, ofRectangle defaultValue) { return has(key) ? ofxToRectangle(keys[key]) : defaultValue; }
ofQuaternion ofxIniSettings::get(string key, ofQuaternion defaultValue) { return has(key) ? ofxToQuaternion(keys[key]) : defaultValue; }
ofMatrix4x4 ofxIniSettings::get(string key, ofMatrix4x4 defaultValue) { return has(key) ? ofxToMatrix4x4(keys[key]) : defaultValue; }

void ofxIniSettings::setString(string newID, string newValue) {
    if (outputFilename=="") ofxExit("ofxIniSettings::setString: outputFilename undefined");
    bool foundKey = false;
    outputFilename = ofToDataPath(outputFilename);
    if (!ofxFileExists(outputFilename)) ofxExit("ofxIniSettings::setString: file not found: " + outputFilename);
    string cmd,section,key,value,id;
    ifstream fileInput(outputFilename.c_str(),ios::in);
    vector<string> lines;

    while (getline(fileInput,cmd)) {
        switch (cmd[0]) {
            case 0: break;   //empty first line?
            case '#': break; //comment
            case ';': break; //comment
            case '[': section = cmd.substr(1,cmd.length()-2); break; //section, remove brackets
            default:
                int pos = cmd.find("=", 0);
                if (pos==string::npos) break;
                key = cmd.substr(0, pos);
                id = section!="" ? (section + "." + key) : key;
                if (id==newID) {
                    foundKey=true;
                    cmd=key+"="+newValue;  //change the value if found
                }
        }
        lines.push_back(cmd); //write to output list
    }
    fileInput.close();
    if (!foundKey) {
        lines.insert(lines.begin(), newID + "=" + newValue);
        ofLogWarning() << "ofxIniSettings: New key '" << newID << "' created in '" << outputFilename << "'";
    }
    ofstream fileOutput(outputFilename.c_str(),ios::out);
    for (int i=0; i<lines.size(); i++) fileOutput << lines[i] << endl;
    fileOutput.close();
}

void ofxIniSettings::print() {
  for (map<string,string>::iterator i=keys.begin(); i!=keys.end() ; i++) {
    cout << i->first << "=" << i->second << endl;
  }
}
