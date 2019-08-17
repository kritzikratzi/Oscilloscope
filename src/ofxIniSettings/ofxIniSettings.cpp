#include "ofxIniSettings.h"
#include "ofxIniExtras.h"

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




void ofxIniSettings::ofxExit(string error ){
	cerr << error << endl;
	exit(1);
}

bool ofxIniSettings::ofxFileExists(string filename) {
	if (filename=="") return false;
	ofFile f(filename);
	return f.exists();
	//	ifstream inp;
	//	inp.open(ofToDataPath(filename).c_str(), ifstream::in);
	//	inp.close();
	//	return !inp.fail();
}

bool ofxIniSettings::ofxStringEndsWith(string str, string key) {
	//http://www.codeproject.com/KB/stl/STL_string_util.aspx?display=Print
	size_t i = str.rfind(key);
	return (i != string::npos) && (i == (str.length() - key.length()));
}

string ofxIniSettings::ofxFormatString(string format, int number) {
	char buffer[100];
	sprintf(buffer, format.c_str(), number);
	return (string)buffer;
}

string ofxIniSettings::ofxFormatString(string format, string s) {
	char buffer[100];
	sprintf(buffer, format.c_str(), s.c_str());
	return (string)buffer;
}

string ofxIniSettings::ofxReplaceString(string input, string replace, string by) {
	ofStringReplace(input, replace, by);
	return input;
}

//void ofxIniSettings::ofxAppendToFile(string filename, string str) {
//    filename = ofToDataPath(filename);
//    if (filename=="") die("ofAppendToFile:no filename");
//    FILE * pFile;
//    pFile = fopen (filename.c_str(),"a");
//    if (pFile==NULL) die("ofAppendToFile:could not open file: " + filename);
//    else fprintf (pFile, str.c_str());
//    fclose(pFile);
//}

void ofxIniSettings::ofxSaveString(string filename, string str) {
	ofstream file(ofToDataPath(filename).c_str(),ios::out);
	file << str;
	file.close();
}

void ofxIniSettings::ofxSaveStrings(string filename, vector<string> lines) {
	ofstream file(ofToDataPath(filename).c_str(),ios::out);
	for (int i=0; i<lines.size(); i++) file << lines[i] << endl;
	file.close();
}

// trim trailing spaces
string ofxIniSettings::ofxTrimStringRight(string str) {
	size_t endpos = str.find_last_not_of(" \t\r\n");
	return (string::npos != endpos) ? str.substr( 0, endpos+1) : str;
}

// trim trailing spaces
string ofxIniSettings::ofxTrimStringLeft(string str) {
	size_t startpos = str.find_first_not_of(" \t\r\n");
	return (string::npos != startpos) ? str.substr(startpos) : str;
}

string ofxIniSettings::ofxTrimString(string str) {
	return ofxTrimStringLeft(ofxTrimStringRight(str));;
}

string ofxIniSettings::ofxStringBeforeFirst(string str, string key) {
	size_t endpos = str.find(key);
	return (string::npos != endpos) ? str.substr(0, endpos) : str;
}

string ofxIniSettings::ofxStringAfterFirst(string str, string key) {
	size_t startpos = str.find(key);
	return (string::npos != startpos) ? str.substr(startpos+key.size()) : str;
}

string ofxIniSettings::ofxStringAfterLast(string str, string key) {
	vector<string> items = ofSplitString(str, key);
	return items.back();
}

string ofxIniSettings::ofxStringBeforeLast(string str, string key) {
	vector<string> items = ofSplitString(str, key);
	items.pop_back();
	return ofJoinString(items, key);
}

bool ofxIniSettings::ofxContains(vector<string> keys, string key) {
	return std::find(keys.begin(), keys.end(), key)!=keys.end();
}

float ofxIniSettings::ofxDist(float ax, float ay, float az, float bx, float by, float bz) {
	return ofVec3f(ax,ay,az).distance(ofVec3f(bx,by,bz));
	//return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
}

bool ofxIniSettings::ofxColorMatch(ofColor a, ofColor b, int tolerance) {
	return ofxDist(a.r,a.g,a.b,b.r,b.g,b.b)<=tolerance;
	//return if (c->color.r==rgb.r && c->color.g==rgb.g && c->color.b==rgb.b) return c;
}

void ofxIniSettings::ofxSetHexColor(int hexColor, int a) { //alpha between 0..255
	int r = (hexColor >> 16) & 0xff;
	int g = (hexColor >> 8) & 0xff;
	int b = (hexColor >> 0) & 0xff;
	ofSetColor(r,g,b,a);
}

void ofxIniSettings::ofxSetColor(ofColor c) {
	ofSetColor(c.r,c.g,c.b,c.a);
}

void ofxIniSettings::ofxSetColorHSB(int h, int s, int b, int a) {
	unsigned char hsv[] = {(unsigned char)h,(unsigned char)s,(unsigned char)b};
	unsigned char rgb[] = {0,0,0};
	
	if (hsv[2] == 0); // black
	else if (hsv[1] == 0) { rgb[0]=b;  rgb[1]=b; rgb[2]=b; } // grays
	else {
		float h = hsv[0] * 6. / 255.;
		float s = hsv[1] / 255.;
		int hCategory = (int) floorf(h);
		float hRemainder = h - hCategory;
		unsigned char pv = (unsigned char) ((1.f - s) * hsv[2]);
		unsigned char qv = (unsigned char) ((1. - s * hRemainder) * hsv[2]);
		unsigned char tv = (unsigned char) ((1. - s * (1.-hRemainder)) * hsv[2]);
		switch (hCategory) {
			case 0: rgb[0] = hsv[2]; rgb[1] = tv; rgb[2] = pv; break; //r
			case 1: rgb[0] = qv; rgb[1] = hsv[2]; rgb[2] = pv; break; //g
			case 2: rgb[0] = pv; rgb[1] = hsv[2]; rgb[2] = tv; break;
			case 3: rgb[0] = pv; rgb[1] = qv; rgb[2] = hsv[2]; break; //b
			case 4: rgb[0] = tv; rgb[1] = pv; rgb[2] = hsv[2]; break;
			case 5: rgb[0] = hsv[2]; rgb[1] = pv; rgb[2] = qv; break; //back to r
		}
	}
	ofSetColor(rgb[0],rgb[1],rgb[2],a);
}

bool ofxIniSettings::ofxToBoolean(string str) { //rick 3-1-2008
	return str=="true" || str=="True" || str=="TRUE" || str=="1";
}

bool ofxIniSettings::ofxToBoolean(float f) {
	return f==0;
}

int ofxIniSettings::ofxToInteger(string str) {
	//works also for hex: 0xff0000....
	istringstream stream(str);
	stream.unsetf(ios_base::dec);
	int result; stream >> result; return result;
}

string ofxIniSettings::ofxToString(char ch){
	stringstream sstr;
	sstr << ch;
	return sstr.str();
}

string ofxIniSettings::ofxToString(unsigned char ch){
	stringstream sstr;
	sstr << ch;
	return sstr.str();
}

string ofxIniSettings::ofxToString(string str){
	return str; //no conversion but for sake of completeness. Is ie. used by template in ofxIniSettings
}

string ofxIniSettings::ofxToString(float f) {
	return ofToString(f);
}

string ofxIniSettings::ofxToString(bool value){
	stringstream sstr;
	sstr << (value ? "true" : "false");
	return sstr.str();
}

string ofxIniSettings::ofxToString(int value) {
	return ofToString(value);
}

string ofxIniSettings::ofxToString(ofRectangle v) {
	return ofToString(v.x) + "," + ofToString(v.y) + "," + ofToString(v.width) + "," + ofToString(v.height);
}

ofColor ofxIniSettings::ofxToColor(int hexColor) {
	ofColor c;
	c.r = (hexColor >> 16) & 0xff;
	c.g = (hexColor >> 8) & 0xff;
	c.b = (hexColor >> 0) & 0xff;
	return c;
}

ofColor ofxIniSettings::ofxToColor(unsigned char r, unsigned char g, unsigned char b) {
	ofColor c;
	c.r = r;
	c.g = g;
	c.b = b;
	return c;
}

ofColor ofxIniSettings::ofxToColor(ofVec4f v) {
	return ofColor(v.x,v.y,v.z,v.w);
}

ofColor ofxIniSettings::ofxToColor(ofVec3f v, int alpha) {
	return ofColor(v.x,v.y,v.z,alpha);
}

string ofxIniSettings::ofxToHexString(int value, int digits) {
	string result;
	char * buf = new char[digits+1];
	string format = "%0"+ofToString(digits)+"x";
	sprintf(buf,format.c_str(),value);
	result = buf;
	delete [] buf;
	return result;
}

int ofxIniSettings::ofxToInteger(ofColor c) {
	return (int(c.r) << 16) + (int(c.g) << 8) + int(c.b);
}

string ofxIniSettings::ofxToString(ofQuaternion q) {
	return ofToString(q.x()) + "," + ofToString(q.y()) + "," + ofToString(q.z()) + "," + ofToString(q.w());
}

ofQuaternion ofxIniSettings::ofxToQuaternion(string str) {
	vector <string> v = ofSplitString(str,",");
	if (v.size()!=4) return ofVec4f(0,0,0,0);
	else return ofVec4f(ofToFloat(v[0]),ofToFloat(v[1]),ofToFloat(v[2]),ofToFloat(v[3]));
}

string ofxIniSettings::ofxToHex(char c) {
	string result;
	char buf[2];
	sprintf(buf,"%02x",c);
	result = buf;
	return result;
}

vector<string> ofxIniSettings::ofxToStringVector(string value) {
	vector<string> lines;
	lines.push_back(value);
	return lines;
}

ofVec2f ofxIniSettings::ofxToVec2f(string str) {
	vector<string> v = ofSplitString(str,",");
	if (v.size()==1) return ofVec2f(ofToFloat(v[0]),ofToFloat(v[0])); ///is dit gewenst?
	if (v.size()!=2) return ofVec2f(0,0);
	else return ofVec2f(ofToFloat(v[0]),ofToFloat(v[1]));
}

ofVec3f ofxIniSettings::ofxToVec3f(string str) {
	vector <string> v = ofSplitString(str,",");
	if (v.size()==1) return ofVec3f(ofToFloat(v[0]),ofToFloat(v[0]),ofToFloat(v[0])); ///is dit gewenst? .5 wordt dus (.5,.5,.5) wordt oa gebruikt voor scale.
	if (v.size()==2) return ofVec3f(ofToFloat(v[0]),ofToFloat(v[1]),0); //2D
	if (v.size()!=3) return ofVec3f(0,0,0);
	else return ofVec3f(ofToFloat(v[0]),ofToFloat(v[1]),ofToFloat(v[2]));
}

ofVec3f ofxIniSettings::ofxToVec3f(float *a) {
	return ofVec3f(a[0],a[1],a[2]);
}


ofVec4f ofxIniSettings::ofxToVec4f(string str) {
	vector <string> v = ofSplitString(str,",");
	if (v.size()!=4) return ofVec4f(0,0,0,0);
	else return ofVec4f(ofToFloat(v[0]),ofToFloat(v[1]),ofToFloat(v[2]),ofToFloat(v[3]));
}

ofRectangle ofxIniSettings::ofxToRectangle(ofVec4f v) {
	return ofRectangle(v.x,v.y,v.z,v.w);
}

ofRectangle ofxIniSettings::ofxToRectangle(string str) {
	return ofxToRectangle(ofxToVec4f(str));
}

string ofxIniSettings::ofxToString(ofVec2f v) {
	return ofToString(v.x) + "," + ofToString(v.y);
}

string ofxIniSettings::ofxToString(ofVec3f v, int precision) {
	return ofToString(v.x,precision) + "," + ofToString(v.y,precision) + "," + ofToString(v.z,precision);
}

string ofxIniSettings::ofxToString(ofVec4f v) {
	return ofToString(v.x) + "," + ofToString(v.y) + "," + ofToString(v.z) + "," + ofToString(v.w);
}

string ofxIniSettings::ofxToString(ofMatrix4x4 m) {
	stringstream ss;
	ss << m;
	string s = ss.str();
	ofStringReplace(s, "\n", ",");
	ofStringReplace(s, " ", "");
	return s;
}

ofMatrix4x4 ofxIniSettings::ofxToMatrix4x4(string s) {
	vector<string> a = ofSplitString(s, ",");
	float mat[16];
	for (int i=0; i<16; i++) {
		mat[i] = ofToFloat(a[i]);
	}
	return ofMatrix4x4(mat);
}

vector<float> ofxIniSettings::ofxToFloatVector(string s, string delimiter) {
	vector<float> f;
	vector<string> items = ofSplitString(s,delimiter);
	for (int i=0; i<items.size(); i++) {
		f.push_back(ofToFloat(items.at(i)));
	}
	return f;
}
