#include <sys/stat.h>
#include "ofxExtras.h"

void ofxNotice(string msg) {
    ofLog(OF_LOG_NOTICE, msg);
}

#ifdef TARGET_OS_X
unsigned int ofxGetFileAge(string filename) {
    struct stat fileinfo;
    int rv = stat(filename.c_str(), &fileinfo);
    if (rv < 0 && errno == ENOENT) { //ignore if file does not exist
        return 0;
    } else if (rv < 0) {
        ofLogError() << "could not stat '" << filename << "' (" << strerror(errno) << ")";
        return -1;
    }
    return ofGetUnixTime() - fileinfo.st_mtimespec.tv_sec;
}
#endif

string ofxGetFileExtension(string filename) {
    int pos = filename.rfind('.');
    if (pos==string::npos) return "";
    return filename.substr(pos);
}

bool ofxFileExists(string filename) {
  if (filename=="") return false;
  ofFile f(filename);
  return f.exists();
  //	ifstream inp;
  //	inp.open(ofToDataPath(filename).c_str(), ifstream::in);
  //	inp.close();
  //	return !inp.fail();
}

string ofxAddTrailingSlash(string foldername) {
    return ofxAddTrailingString(foldername,"/");
}

string ofxAddTrailingString(string str, string trail) {
    if (str.length()==0) return "";
    if (!ofxStringEndsWith(str,trail)) str+=trail;
    return str;
}

bool ofxStringEndsWith(string str, string key) {
    //http://www.codeproject.com/KB/stl/STL_string_util.aspx?display=Print
    size_t i = str.rfind(key);
    return (i != string::npos) && (i == (str.length() - key.length()));
}

bool ofxStringStartsWith(string str, string key) {
    return str.find(key) == 0;
}

void ofxExit(string str) {
    cout << str << endl;
    ofxExit();
}

void ofxExit() {
    std::exit(1);
}

string ofxFormatString(string format, int number) {
    char buffer[100];
	sprintf(buffer, format.c_str(), number);
	return (string)buffer;
}

string ofxFormatString(string format, string s) {
    char buffer[100];
	sprintf(buffer, format.c_str(), s.c_str());
	return (string)buffer;
}

string ofxReplaceString(string input, string replace, string by) {
    ofStringReplace(input, replace, by);
    return input;
}

//void ofxAppendToFile(string filename, string str) {
//    filename = ofToDataPath(filename);
//    if (filename=="") die("ofAppendToFile:no filename");
//    FILE * pFile;
//    pFile = fopen (filename.c_str(),"a");
//    if (pFile==NULL) die("ofAppendToFile:could not open file: " + filename);
//    else fprintf (pFile, str.c_str());
//    fclose(pFile);
//}

string ofxUrlToSafeLocalPath(string url) {
    string filename = url;
    filename = ofToLower(filename);
    filename = ofxTrimString(filename);
    filename = ofxReplaceString(filename, "http://", "");
    filename = ofxReplaceString(filename, "/", "-");
    filename = ofxReplaceString(filename, " ", "_");
    filename = ofxReplaceString(filename, ":", ".");
    //filename = "images/"+filename;
    return filename;
}

string ofxGetFilenameFromUrl(string url) {
    vector<string> items = ofSplitString(url,"/");
    if (items.size()==0) return url;
    else return items.back();
}

vector<string> ofxLoadStrings(string url) {
    using Poco::URI;
    URI uri(url);

    if (uri.isRelative() || !ofxStringStartsWith(url,"http")) {
        string filename = url; //uri.getPathAndQuery();
        vector<string> lines;
        filename = ofToDataPath(filename);
        if (!ofxFileExists(filename)) { ofLogError() << "ofxLoadStrings: File not found: " << filename; return lines; }
        ifstream f(filename.c_str(),ios::in);
        string line;
        while (getline(f,line)) lines.push_back(ofxTrimStringRight(line));
        f.close();
        return lines;
    } else {
		cerr << "http support disabled because of VS express" << endl; 
		return vector<string>(); 
        /*try {
            string str;
            Poco::Net::HTTPClientSession session(uri.getHost());
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
            vector<string> usernamePassword = ofSplitString(uri.getUserInfo(),":");
            if (usernamePassword.size()==2) {
                Poco::Net::HTTPBasicCredentials auth(usernamePassword[0],usernamePassword[1]);
                auth.authenticate(request);
            }
            session.sendRequest(request);
            Poco::Net::HTTPResponse response;
            istream& rs = session.receiveResponse(response);

            if (response.getStatus() == 200) {
                Poco::StreamCopier::copyToString(rs, str);
                return ofSplitString(str,"\n",true,true);
            } else {
                ofLogError() << ("ofxLoadStrings: HTTP Error " + ofxToString(response.getStatus()));
                vector<string> lines;
                return lines;
            }
        }  catch (Poco::Exception &e) {
            ofxExit("ofxLoadStrings: Problem loading data: " + e.displayText() + " - " + url);
        }*/
    }
}

void ofxSaveString(string filename, string str) {
    ofstream file(ofToDataPath(filename).c_str(),ios::out);
    file << str;
    file.close();
}

void ofxSaveStrings(string filename, vector<string> lines) {
    ofstream file(ofToDataPath(filename).c_str(),ios::out);
    for (int i=0; i<lines.size(); i++) file << lines[i] << endl;
    file.close();
}

string ofxGetIsoDateTime() {
    return ofxFormatDateTime(ofxGetDateTime(), "%Y-%m-%d %X");
}

string ofxFormatDateTime(time_t rawtime, string format) {
	char buffer[80];
	struct tm *timeinfo = localtime(&rawtime);
	strftime(buffer,80,format.c_str(),timeinfo);
	return (string)buffer;
}

time_t ofxParseDateTime(string datetime, string format) {
    //http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    ofLogError("ofxParseDateTime not implemented in this version of ofxExtras");
    //    struct tm tm[1] = {{0}};
    //    strptime(datetime.c_str(), format.c_str(), tm);
    //    return mktime(tm);
	return 0; 
}

//vector<string> ofxParseString(string str, string format) {
//    // Example:
//    string input = "foo bar 10 20 hello 30";
//    int a,b,c;
//    string garbage;
//    stringstream foo;
//    foo << input;
//    input >> garbage >> garbage >> a >> b >> garbage >> c;
//}

time_t ofxGetDateTime() {
    time_t rawtime;
    time(&rawtime);
    return rawtime;
}

void ofxSetTexture(ofBaseHasTexture &material) {
    material.getTextureReference().bind();
}

void ofxSetTexture(ofTexture &texture) {
    texture.bind();
}

void ofxDisableTexture() {
    glDisable(GL_TEXTURE_2D);
#ifdef GL_TEXTURE_RECTANGLE_ARB
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
#endif

}

// trim trailing spaces
string ofxTrimStringRight(string str) {
    size_t endpos = str.find_last_not_of(" \t\r\n");
    return (string::npos != endpos) ? str.substr( 0, endpos+1) : str;
}

// trim trailing spaces
string ofxTrimStringLeft(string str) {
    size_t startpos = str.find_first_not_of(" \t\r\n");
    return (string::npos != startpos) ? str.substr(startpos) : str;
}

string ofxTrimString(string str) {
	return ofxTrimStringLeft(ofxTrimStringRight(str));;
}

string ofxStringBeforeFirst(string str, string key) {
	size_t endpos = str.find(key);
	return (string::npos != endpos) ? str.substr(0, endpos) : str;
}

string ofxStringAfterFirst(string str, string key) {
	size_t startpos = str.find(key);
	return (string::npos != startpos) ? str.substr(startpos+key.size()) : str;
}

string ofxStringAfterLast(string str, string key) {
  vector<string> items = ofSplitString(str, key);
  return items.back();
}

string ofxStringBeforeLast(string str, string key) {
  vector<string> items = ofSplitString(str, key);
  items.pop_back();
  return ofJoinString(items, key);
}

bool ofxContains(vector<string> keys, string key) {
    return std::find(keys.begin(), keys.end(), key)!=keys.end();
}

float ofxDist(float ax, float ay, float az, float bx, float by, float bz) {
    return ofVec3f(ax,ay,az).distance(ofVec3f(bx,by,bz));
    //return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
}

bool ofxColorMatch(ofColor a, ofColor b, int tolerance) {
	return ofxDist(a.r,a.g,a.b,b.r,b.g,b.b)<=tolerance;
	//return if (c->color.r==rgb.r && c->color.g==rgb.g && c->color.b==rgb.b) return c;
}

void ofxScale(float scale) {
    ofScale(scale,scale,scale);
}

string ofxGetHostName() {
    string tmpFile = ofToDataPath("hostname.txt");
    system(("hostname > " + tmpFile).c_str());
    string hostname = ofxLoadStrings(tmpFile)[0];
    system(("rm " + tmpFile).c_str());
    return hostname;
}

void ofxSetHexColor(int hexColor, int a) { //alpha between 0..255
	int r = (hexColor >> 16) & 0xff;
	int g = (hexColor >> 8) & 0xff;
	int b = (hexColor >> 0) & 0xff;
	ofSetColor(r,g,b,a);
}

void ofxSetColor(ofColor c) {
	ofSetColor(c.r,c.g,c.b,c.a);
}

void ofxSetColorHSB(int h, int s, int b, int a) {
    unsigned char hsv[] = {h,s,b};
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

bool ofxToBoolean(string str) { //rick 3-1-2008
	return str=="true" || str=="True" || str=="TRUE" || str=="1";
}

bool ofxToBoolean(float f) {
	return f==0;
}

int ofxToInteger(string str) {
    //works also for hex: 0xff0000....
    istringstream stream(str);
    stream.unsetf(ios_base::dec);
    int result; stream >> result; return result;
}

string ofxToString(char ch){
	stringstream sstr;
	sstr << ch;
	return sstr.str();
}

string ofxToString(unsigned char ch){
	stringstream sstr;
	sstr << ch;
	return sstr.str();
}

string ofxToString(string str){
	return str; //no conversion but for sake of completeness. Is ie. used by template in ofxIniSettings
}

string ofxToString(float f) {
    return ofToString(f);
}

string ofxToString(bool value){
	stringstream sstr;
	sstr << (value ? "true" : "false");
	return sstr.str();
}

string ofxToString(int value) {
    return ofToString(value);
}

string ofxToString(ofRectangle v) {
    return ofToString(v.x) + "," + ofToString(v.y) + "," + ofToString(v.width) + "," + ofToString(v.height);
}

ofColor ofxToColor(int hexColor) {
	ofColor c;
	c.r = (hexColor >> 16) & 0xff;
	c.g = (hexColor >> 8) & 0xff;
	c.b = (hexColor >> 0) & 0xff;
	return c;
}

ofColor ofxToColor(unsigned char r, unsigned char g, unsigned char b) {
	ofColor c;
	c.r = r;
	c.g = g;
	c.b = b;
	return c;
}

ofColor ofxToColor(ofVec4f v) {
	return ofColor(v.x,v.y,v.z,v.w);
}

ofColor ofxToColor(ofVec3f v, int alpha) {
	return ofColor(v.x,v.y,v.z,alpha);
}

string ofxToHexString(int value, int digits=6) {
	string result;
    char * buf = new char[digits+1];
	string format = "%0"+ofToString(digits)+"x";
    sprintf(buf,format.c_str(),value);
    result = buf;
	delete buf;
    return result;
}

string ofxToString(ofColor c) {
	return ofxToHexString(ofxToInteger(c));
}

int ofxToInteger(ofColor c) {
	return (int(c.r) << 16) + (int(c.g) << 8) + int(c.b);
}

string ofxToString(ofQuaternion q) {
    return ofToString(q.x()) + "," + ofToString(q.y()) + "," + ofToString(q.z()) + "," + ofToString(q.w());
}

ofQuaternion ofxToQuaternion(string str) {
    vector <string> v = ofSplitString(str,",");
    if (v.size()!=4) return ofVec4f(0,0,0,0);
    else return ofVec4f(ofToFloat(v[0]),ofToFloat(v[1]),ofToFloat(v[2]),ofToFloat(v[3]));
}

string ofxToHex(char c) {
    string result;
    char buf[2];
    sprintf(buf,"%02x",c);
    result = buf;
    return result;
}

vector<string> ofxToStringVector(string value) {
    vector<string> lines;
    lines.push_back(value);
    return lines;
}

void ofxRotate(ofVec3f v) {
    glRotatef(v.x,1,0,0);
    glRotatef(v.y,0,1,0);
    glRotatef(v.z,0,0,1);
}

void ofxRotate(ofQuaternion q) {
	//rotation
	float angle;
	ofVec3f axis;
	q.getRotate(angle, axis);
	ofRotate(angle/TWO_PI*360,axis.x,axis.y,axis.z);
}

void ofxRotate(ofNode &node, ofQuaternion q) {
    node.setOrientation(node.getOrientationQuat() * q);
}

void ofxRotate(float angle, ofVec3f v) {
    glRotatef(angle,v.x,v.y,v.z);
}

void ofxTranslate(ofVec3f v) {
    glTranslatef(v.x,v.y,v.z);
}

void ofxScale(ofVec3f v) {
    glScalef(v.x,v.y,v.z);
}

ofVec2f ofxToVec2f(string str) {
    vector<string> v = ofSplitString(str,",");
    if (v.size()==1) return ofVec2f(ofToFloat(v[0]),ofToFloat(v[0])); ///is dit gewenst?
    if (v.size()!=2) return ofVec2f(0,0);
    else return ofVec2f(ofToFloat(v[0]),ofToFloat(v[1]));
}

ofVec3f ofxToVec3f(string str) {
    vector <string> v = ofSplitString(str,",");
    if (v.size()==1) return ofVec3f(ofToFloat(v[0]),ofToFloat(v[0]),ofToFloat(v[0])); ///is dit gewenst? .5 wordt dus (.5,.5,.5) wordt oa gebruikt voor scale.
    if (v.size()==2) return ofVec3f(ofToFloat(v[0]),ofToFloat(v[1]),0); //2D
    if (v.size()!=3) return ofVec3f(0,0,0);
    else return ofVec3f(ofToFloat(v[0]),ofToFloat(v[1]),ofToFloat(v[2]));
}

ofVec3f ofxToVec3f(float *a) {
    return ofVec3f(a[0],a[1],a[2]);
}


ofVec4f ofxToVec4f(string str) {
    vector <string> v = ofSplitString(str,",");
    if (v.size()!=4) return ofVec4f(0,0,0,0);
    else return ofVec4f(ofToFloat(v[0]),ofToFloat(v[1]),ofToFloat(v[2]),ofToFloat(v[3]));
}

ofRectangle ofxToRectangle(string str) {
    return ofxToRectangle(ofxToVec4f(str));
}

ofRectangle ofxToRectangle(ofVec4f v) {
    return ofRectangle(v.x,v.y,v.z,v.w);
}

string ofxToString(ofVec2f v) {
    return ofToString(v.x) + "," + ofToString(v.y);
}

string ofxToString(ofVec3f v, int precision) {
    return ofToString(v.x,precision) + "," + ofToString(v.y,precision) + "," + ofToString(v.z,precision);
}

string ofxToString(ofVec4f v) {
    return ofToString(v.x) + "," + ofToString(v.y) + "," + ofToString(v.z) + "," + ofToString(v.w);
}

string ofxToString(ofMatrix4x4 m) {
    stringstream ss;
    ss << m;
    string s = ss.str();
    ofStringReplace(s, "\n", ",");
    ofStringReplace(s, " ", "");
    return s;
}

ofMatrix4x4 ofxToMatrix4x4(string s) {
    vector<string> a = ofSplitString(s, ",");
    float mat[16];
    for (int i=0; i<16; i++) {
        mat[i] = ofToFloat(a[i]);
    }
    return ofMatrix4x4(mat);
}

vector<float> ofxToFloatVector(string s, string delimiter) {
  vector<float> f;
  vector<string> items = ofSplitString(s,delimiter);
  for (int i=0; i<items.size(); i++) {
    f.push_back(ofToFloat(items.at(i)));
  }
  return f;
}

void ofxQuadricSphere(float radius, int resolution) {
#ifndef TARGET_OPENGLES
    static GLUquadricObj *quadric = gluNewQuadric(); //because it's static, it's created only once
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluSphere(quadric, radius, resolution, resolution);
#endif
}

void ofxQuadricDisk(float innerRadius, float outerRadius, int resolution) {
#ifndef TARGET_OPENGLES
    static GLUquadricObj *quadric = gluNewQuadric(); //because it's static, it's created only once
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluDisk(quadric, innerRadius, outerRadius, resolution, resolution);
#endif
}

void ofxDrawDisk(ofBaseHasTexture &img,float r, float slices) {
#ifndef TARGET_OPENGLES
    float cx = img.getTextureReference().getWidth()/2; //center of image
    float cy = img.getTextureReference().getHeight()/2; //center of image
    float step = TWO_PI/slices; //size of a slice in radians

    img.getTextureReference().bind();

    glBegin(GL_TRIANGLE_FAN);
    for (float f=0; f<TWO_PI; f+=step) {
        glTexCoord2f(cx,cy);
        glVertex2f(0,0);
        glTexCoord2f(cx+cx*sin(f), cy+cy*cos(f));
        glVertex2f(r*sin(f), r*cos(f));
        glTexCoord2f(cx+cx*sin(f+step), cy+cy*cos(f+step));
        glVertex2f(r*sin(f+step), r*cos(f+step));
    }
    glEnd();
#endif
}

void ofxEnableDepthTest() {
    glEnable(GL_DEPTH_TEST);
}

void ofxDisableDepthTest() {
    glDisable(GL_DEPTH_TEST);
}

string ofxGetSerialString(ofSerial &serial, char until) {
    static string str;
    stringstream ss;
    char ch;
    int ttl=1000;
    while ((ch=serial.readByte())>0 && ttl-->0 && ch!=until) {
        //if (ch==OF_SERIAL_ERROR) return "OF_SERIAL_ERROR";
        ss << ch;
    }
    str+=ss.str();
    if (ch==until) {
        string tmp=str;
        str="";
        return ofxTrimString(tmp);
    } else {
        return "";
    }
}

string ofxGetSerialString2(ofSerial &serial, char until) {
    static string str;
    stringstream ss;
    char ch;
    int ttl=1000;
    while ((ch=serial.readByte())>0 && ttl-->0 && ch!=until) {
        //if (ch==OF_SERIAL_ERROR) return "OF_SERIAL_ERROR";
        ss << ch;
    }
    str+=ss.str();
    if (ch==until) {
        string tmp=str;
        str="";
        return ofxTrimString(tmp);
    } else {
        return "";
    }
}

//
//bool ofxGetSerialString(ofSerial &serial, string &output_str, char until) {
//    static string tmpstr; //cannot use output_str unless it's a member var of testApp. we want also support for local vars in functions. OR, we can try if this is really the case and needed
//    stringstream ss;
//    char ch;
//    int ttl=1000;
//    while ((ch=serial.readByte())>0 && ttl-->0 && ch!=until) {
//        ss << ch;
//    }
//    tmpstr+=ss.str();
//    if (ch==until) {
//        output_str = tmpstr;
//        tmpstr = "";
//    }
//    return tmpstr!="";
//}
//
//void ofxSerialWrite(ofSerial &serial, string str) {
//    serial.writeBytes((unsigned char*)str.c_str(), str.size());
//}
//
//void ofxSerialWriteLine(ofSerial &serial, string str) {
//    str+="\n";
//    serial.writeBytes((unsigned char*)str.c_str(), str.size());
//}


/*string ofxGetSerialString(ofSerial &serial, char until) {
#ifndef TARGET_OPENGLES
    static string str;
    stringstream ss;
    char ch;
    int ttl=1000;
    while ((ch=serial.readByte())>0 && ttl-->0 && ch!=until) {
        //if (ch==OF_SERIAL_ERROR) return "OF_SERIAL_ERROR";
        ss << ch;
    }
    str+=ss.str();
    if (ch==until) {
        string tmp=str;
        str="";
        return ofxTrimString(tmp);
    } else {
        return "";
    }
#endif
}

bool ofxGetSerialString(ofSerial &serial, string &output_str, char until) {
#ifndef TARGET_OPENGLES
    static string tmpstr; //cannot use output_str unless it's a member var of testApp. we want also support for local vars in functions. OR, we can try if this is really the case and needed
    stringstream ss;
    char ch;
    int ttl=1000;
    while ((ch=serial.readByte())>0 && ttl-->0 && ch!=until) {
        ss << ch;
    }
    tmpstr+=ss.str();
    if (ch==until) {
        output_str = tmpstr;
        tmpstr = "";
    }
    return tmpstr!="";
#endif
}

void ofxSerialWrite(ofSerial &serial, string str) {
#ifndef TARGET_OPENGLES
    serial.writeBytes((unsigned char*)str.c_str(), str.size());
#endif
}

void ofxSerialWriteLine(ofSerial &serial, string str) {
#ifndef TARGET_OPENGLES
    str+="\n";
    serial.writeBytes((unsigned char*)str.c_str(), str.size());
#endif
}
*/

ofVec3f ofxMouseToSphere(float x, float y) {  //-0.5 ... +0.5
    ofVec3f v(x,y);
    float mag = v.x*v.x + v.y*v.y;
    if (mag>1.0f) v.normalize();
    else v.z = sqrt(1.0f - mag);
    return v;
}

ofVec3f ofxMouseToSphere(ofVec2f v) {  //-0.5 ... +0.5
    return ofxMouseToSphere(v.x,v.y);
}

void ofxSetWindowRect(ofRectangle window) {
    ofSetWindowPosition(window.x,window.y);
    ofSetWindowShape(window.width, window.height);
}

bool ofxMouseMoved() {
    static ofPoint pmouse;
    ofPoint mouse(ofGetMouseX(),ofGetMouseY());
    bool mouseIsMoving = (mouse!=pmouse);
    pmouse = mouse;
    return mouseIsMoving;
}

void ofxSetCursor(bool bVisible) {
    bVisible ? ofShowCursor() : ofHideCursor();
}

float ofxGetHeading(ofPoint p, ofPoint anchor) { //radians
    p-=anchor;
    return (float)atan2(p.y, -p.x)+PI;
}

ofPoint ofxGetPointOnCircle(float angle, float radius) { //radians
    ofPoint p;
    p.x = radius*cos(angle);
    p.y = radius*sin(-angle);
    return p;
}

int ofxMakeEven(int v, int add) {
    return (v%2==0) ? v : v+add;
}

int ofxMakeOdd(int v, int add) {
    return (v%2==0) ? v+add : v;
}

ofPoint ofxGetMouse() {
    return ofPoint(ofGetMouseX(),ofGetMouseY());
}

ofPoint ofxGetMouseFromCenter() {
    return ofxGetMouse()-ofxGetCenter();
}

ofPoint ofxGetPreviousMouse() {
    return ofPoint(ofGetPreviousMouseX(), ofGetPreviousMouseY());
}

int ofxIndex(float x, float y, float w) {
    return y*w+x;
}

ofPoint ofxLerp(ofPoint start, ofPoint end, float amt) {
    return start + amt * (end - start);
}

float ofxLerp(float start, float end, float amt) {
    return start + amt * (end - start);
}

void ofxQuadWarp(ofBaseHasTexture &tex, ofPoint lt, ofPoint rt, ofPoint rb, ofPoint lb, int rows, int cols) {
    float tw = tex.getTextureReference().getWidth();
    float th = tex.getTextureReference().getHeight();

    ofMesh mesh;

    for (int x=0; x<=cols; x++) {
        float f = float(x)/cols;
        ofPoint vTop(ofxLerp(lt,rt,f));
        ofPoint vBottom(ofxLerp(lb,rb,f));
        ofPoint tTop(ofxLerp(ofPoint(0,0),ofPoint(tw,0),f));
        ofPoint tBottom(ofxLerp(ofPoint(0,th),ofPoint(tw,th),f));

        for (int y=0; y<=rows; y++) {
            float f = float(y)/rows;
            ofPoint v = ofxLerp(vTop,vBottom,f);
            mesh.addVertex(v);
            mesh.addTexCoord(ofxLerp(tTop,tBottom,f));
        }
    }

    for (float y=0; y<rows; y++) {
        for (float x=0; x<cols; x++) {
            mesh.addTriangle(ofxIndex(x,y,cols+1), ofxIndex(x+1,y,cols+1), ofxIndex(x,y+1,cols+1));
            mesh.addTriangle(ofxIndex(x+1,y,cols+1), ofxIndex(x+1,y+1,cols+1), ofxIndex(x,y+1,cols+1));
        }
    }

    tex.getTextureReference().bind();
    mesh.draw();
    tex.getTextureReference().unbind();
    mesh.drawVertices();
}

void ofxResetTransform(ofNode &node) {
    node.resetTransform();
    node.setScale(1,1,1);
}

void ofxAssert(bool condition, string message) {
    if (!condition) {
        ofLog(OF_LOG_ERROR,"%s", message.c_str());
        std::exit(1);
    }
}


void ofxArcStrip(float innerRadius, float outerRadius, float startAngle, float stopAngle) {  //radians
#ifndef TARGET_OPENGLES
    float delta = fabs(stopAngle-startAngle);
    if (delta<.00001) return; //don't draw if arc to small
    int n = 200 * delta/TWO_PI; //a full circle=200 segments
    if (n==0) return;
    glBegin(GL_TRIANGLE_STRIP); //GL_TRIANGLE_STRIP); //change to GL_LINE_LOOP);  for hollow
    for (int i=0; i<=n; i++) {
        float f = -ofMap(i,0,n,startAngle,stopAngle);
        float x1 = innerRadius * cos(f);
        float y1 = innerRadius * sin(f);
        float x2 = outerRadius * cos(f);
        float y2 = outerRadius * sin(f);
        glVertex2f(x1,y1);
        glVertex2f(x2,y2);
    }
    glEnd();
#endif
}

void ofxArc(float radius, float startAngle, float stopAngle, int detail) { //radians
#ifndef TARGET_OPENGLES
    glBegin(GL_LINE_STRIP);
    for (int i=0,n=detail; i<=n; i++) {
        float f = -ofMap(i,0,n,startAngle,stopAngle);
        float x = radius * cos(f);
        float y = radius * sin(f);
        glVertex2f(x,y);
    }
    glEnd();
#endif
}

// from Cinder
// http://local.wasp.uwa.edu.au/~pbourke/texture_colour/spheremap/  Paul Bourke's sphere code
// We should weigh an alternative that reduces the batch count by using GL_TRIANGLES instead
void ofxDrawSphere(float radius, int segments )
{
	if( segments < 0 )
		return;

	float *verts = new float[(segments+1)*2*3];
	float *normals = new float[(segments+1)*2*3];
	float *texCoords = new float[(segments+1)*2*2];

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, verts );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );
	glEnableClientState( GL_NORMAL_ARRAY );
	glNormalPointer( GL_FLOAT, 0, normals );

	for( int j = 0; j < segments / 2; j++ ) {
		float theta1 = j * 2 * 3.14159f / segments - ( 3.14159f / 2.0f );
		float theta2 = (j + 1) * 2 * 3.14159f / segments - ( 3.14159f / 2.0f );

		for( int i = 0; i <= segments; i++ ) {
			ofVec3f e, p;
			float theta3 = i * 2 * 3.14159f / segments;

			e.x = cos( theta1 ) * cos( theta3 );
			e.y = sin( theta1 );
			e.z = cos( theta1 ) * sin( theta3 );
			p = e * radius; // + center;
			normals[i*3*2+0] = e.x; normals[i*3*2+1] = e.y; normals[i*3*2+2] = e.z;
			texCoords[i*2*2+0] = 0.999f - i / (float)segments; texCoords[i*2*2+1] = 0.999f - 2 * j / (float)segments;
			verts[i*3*2+0] = p.x; verts[i*3*2+1] = p.y; verts[i*3*2+2] = p.z;

			e.x = cos( theta2 ) * cos( theta3 );
			e.y = sin( theta2 );
			e.z = cos( theta2 ) * sin( theta3 );
			p = e * radius; // + center;
			normals[i*3*2+3] = e.x; normals[i*3*2+4] = e.y; normals[i*3*2+5] = e.z;
			texCoords[i*2*2+2] = 0.999f - i / (float)segments; texCoords[i*2*2+3] = 0.999f - 2 * ( j + 1 ) / (float)segments;
			verts[i*3*2+3] = p.x; verts[i*3*2+4] = p.y; verts[i*3*2+5] = p.z;
		}
		glDrawArrays( GL_TRIANGLE_STRIP, 0, (segments + 1)*2 );
	}

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );

	delete [] verts;
	delete [] normals;
	delete [] texCoords;
}

ofPoint ofxGetCenter() {
    return ofPoint(ofGetWidth()/2, ofGetHeight()/2);
}

ofPoint ofxGetCenterOfMass(vector<ofPoint*> points) {
    //calculating the average of the vectors
    ofPoint sum;
    if (points.size()==0) return ofPoint(0,0);
    for (int i=0; i<points.size(); i++) sum+=(*points[i]);
    return sum/points.size();
}

ofRectangle ofxGetBoundingBox(vector<ofPoint*> points) {
    if (points.size()<1) return ofRectangle();
    float xMin=9999,xMax=-9999,yMin=9999,yMax=-9999;
    for (int i=0; i<points.size(); i++) {
        ofPoint &pt = *points[i];
        xMin = min(xMin,pt.x);
        xMax = max(xMax,pt.x);
        yMin = min(yMin,pt.y);
        yMax = max(yMax,pt.y);
    }
    return ofRectangle(xMin,yMin,xMax-xMin,yMax-yMin);
}

ofRectangle ofxGetBoundingBox(vector<ofPoint> &points) {
  if (points.size()<1) return ofRectangle();
  float xMin=9999,xMax=-9999,yMin=9999,yMax=-9999;
  for (int i=0; i<points.size(); i++) {
    ofPoint &pt = points[i];
    xMin = min(xMin,pt.x);
    xMax = max(xMax,pt.x);
    yMin = min(yMin,pt.y);
    yMax = max(yMax,pt.y);
  }
  return ofRectangle(xMin,yMin,xMax-xMin,yMax-yMin);
}

ofRectangle ofxScaleRectangle(ofRectangle rect, float s) {
    return ofRectangle(rect.x*s,rect.y*s,rect.width*s,rect.height*s);
}

////of008 has no subpaths
//void ofxSimplifyPath(ofPath &path, int iterations, float amount, float distance) { //wat doet amount?? should be distance???
//    for (int iteration=0; iteration<iterations; iteration++) {
//        vector<ofSubPath> &subpaths = path.getSubPaths();
//        for (int i=0; i<subpaths.size(); i++) {
//            vector<ofSubPath::Command> &commands = subpaths[i].getCommands();
//            if (commands.size()<amount) continue;
//            for (int j=1; j<commands.size()-2; j++) { //laat eerste en laatste punt met rust
//                if (commands[j].to.distance(commands[j+1].to)<distance) {
//                    commands[j].to = (commands[j].to+commands[j+1].to)/2;
//                    commands.erase(commands.begin()+j+1);
//                }
//            }
//        }
//    }
//    path.flagShapeChanged();
//}

//vector<ofPolyline> ofxGetPolylinesFromPath(ofPath path) {
//    vector<ofPolyline> polylines;
//    vector<ofSubPath> &subpaths = path.getSubPaths();
//    for (int i=0; i<subpaths.size(); i++) {
//        ofPolyline poly;
//        vector<ofSubPath::Command> &commands = subpaths[i].getCommands();
//        for (int j=0; j<commands.size()-1; j++) {
//            poly.addVertex(commands[i].to);
//        }
//        polylines.push_back(poly);
//    }
//    return polylines;
//}
//
//vector<ofPoint*> ofxGetPointsFromPath(ofPath &path) {
//    vector<ofPoint*> points;
//    vector<ofSubPath> &subpaths = path.getSubPaths();
//    for (int i=0; i<subpaths.size(); i++) {
//        vector<ofSubPath::Command> &commands = subpaths[i].getCommands();
//        for (int j=0; j<commands.size(); j++) {
//            points.push_back(&commands[j].to);
//        }
//    }
//    return points;
//}

ofQuaternion ofxToQuaternion(ofxLatLon ll) {
    return ofxToQuaternion(ll.lat, ll.lon);
}

ofQuaternion ofxToQuaternion(float lat, float lon) {
    ofQuaternion q;
    q *= ofQuaternion(lat, ofVec3f(1,0,0));
    q *= ofQuaternion(lon, ofVec3f(0,1,0));
    return q;
}

ofVec3f ofxToCartesian(float lat, float lon) {
    ofVec3f v(0,0,1);
    v.rotate(lat,ofVec3f(1,0,0));
    v.rotate(lon,ofVec3f(0,1,0));
    return v;
}

ofVec3f ofxToCartesian(ofQuaternion q) {
    float angle;
    ofVec3f vec;
    q.getRotate(angle, vec);
    return ofVec3f(0,0,1).rotated(angle, vec);
}

void ofxDrawVertex(ofVec3f v) {
#ifndef TARGET_OPENGLES
    glVertex3f(v.x,v.y,v.z);
#endif
}

ofxLatLon ofxToLatLon(ofQuaternion q) {
    ofVec3f c;
    ofVec4f v(0,0,-1,0);
    ofMatrix4x4 m;
    q.get(m);
    ofVec4f mv = m*v;
    c.set(mv.x,mv.y,-mv.z);
    c.rotate(90, 0, 0);
    float lat = ofRadToDeg(asin(c.z));
    float lon = ofRadToDeg(-atan2(c.y,c.x))-90;
    if (lon<-180) lon+=360;
    return ofxLatLon(lat,lon);
}

ofxLatLon ofxToLatLon(string s) {
  ofVec2f v = ofxToVec2f(s);
	ofxLatLon ll(v.x,v.y);
//  ll.lat = v.x;
//  ll.lon = v.y;
  return ll;
}

string ofxWordWrap(string input, int maxWidth, ofTrueTypeFont *font) {
    vector<string> lines = ofSplitString(input,"\n");
    for (int l=0; l<lines.size(); l++) {
        vector<string> words = ofSplitString(lines[l]," ");
        int strWidth=0;
        for (int w=0; w<words.size(); w++) {
            int nextWidth = font ? font->stringWidth(words[w]+"i") : words[w].length()+1;

            if (strWidth+nextWidth < maxWidth) {
                strWidth+=nextWidth;
            } else {
                strWidth=nextWidth;
                words[w] = "\n" + words[w];
            }
        }
        lines[l] = ofJoinString(words, " ");
    }
    return ofJoinString(lines, "\n");
}

int ofxGetMultiByteStringLength(string s) { //corrected for 3 bytes UTF-8 characters
    //count the number of special chars (3 bytes) in the string
    int total = 0;
    for (int i=0; i<s.length(); i++) {
        if (s.at(i)<0) total++;
    }
    total/=3; //3 bytes per special char
    return s.length()-total*2; //subtract 2 of the length for each special char
}

ofMesh ofxCreateGeoSphere(int stacks, int slices) {
    ofMesh mesh;

    //add vertices
    mesh.addVertex(ofVec3f(0,0,1));

    for (int i=1; i<stacks; i++) {
        double phi = PI * double(i)/stacks;
        double cosPhi = cos(phi);
        double sinPhi = sin(phi);
        for (int j=0; j<slices; j++) {
            double theta = TWO_PI * double(j)/slices;
            mesh.addVertex(ofVec3f(cos(theta)*sinPhi, sin(theta)*sinPhi, cosPhi));
        }
    }
    mesh.addVertex(ofVec3f(0,0,-1));

    //top row triangle fan
    for (int j=1; j<slices; j++) {
        mesh.addTriangle(0,j,j+1);
    }
    mesh.addTriangle(0,slices,1);

    //triangle strips
    for (int i=0; i < stacks-2; i++) {
        int top = i*slices + 1;
        int bottom = (i+1)*slices + 1;

        for (int j=0; j<slices-1; j++) {
            mesh.addTriangle(bottom+j, bottom+j+1, top+j+1);
            mesh.addTriangle(bottom+j, top+j+1, top+j);
        }

        mesh.addTriangle(bottom+slices-1, bottom, top);
        mesh.addTriangle(bottom+slices-1, top, top+slices-1);
    }

    //bottom row triangle fan
    int last = mesh.getNumVertices()-1;
    for (int j=last-1; j>last-slices; j--) {
        mesh.addTriangle(last,j,j-1);
    }
    mesh.addTriangle(last,last-slices,last-1);

    return mesh;
}

void ofxAutoColorMesh(ofMesh &mesh) {
    for (int i=0; i<mesh.getNumVertices(); i++) {
        ofVec3f v = mesh.getVertex(i).normalized();
        mesh.addColor(ofFloatColor(v.x,v.y,v.z));
    }
}

bool ofxOnTimeIntervalSeconds(int s) {
    int fps = 30; //ofGetFrameRate should be constant for this to work well
    return (ofGetFrameNum() % (s*fps) == 0);
}

bool ofxIsWindows() {
    #ifdef WIN32
    return true;
    #else
    return false;
    #endif
}
//
//template<typename T> T ofxFromList(vector<T> &list, float normIndex) {
//    int index = ofClamp(normIndex * list.size(), 0,list.size()-1);
//    if (list.size()==0) return T();
//    return list[index];
//}

bool lexicalComparison(const ofPoint& v1, const ofPoint& v2) {
  if (v1.x > v2.x) return true;
  else if (v1.x < v2.x) return false;
  else if (v1.y > v2.y) return true;
  else return false;
}

bool isRightTurn(ofPoint a, ofPoint b, ofPoint c) {
    // use the cross product to determin if we have a right turn
    return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) >= 0;
}

//vector<ofPoint>
ofPolyline ofxGetConvexHull(vector<ofPoint> points) {
  if (points.size()<3) return ofPolyline();

  ofPoint h1,h2,h3;

  sort(points.begin(), points.end(), lexicalComparison);

  vector<ofPoint> hull;

  hull.push_back(points.at(0));
  hull.push_back(points.at(1));

  int currentPoint = 2;
  int direction = 1;

  for (int i=0; i<1000; i++) { //max 1000 tries

    hull.push_back(points.at(currentPoint));

    // look at the turn direction in the last three points
    h1 = hull.at(hull.size()-3);
    h2 = hull.at(hull.size()-2);
    h3 = hull.at(hull.size()-1);

    // while there are more than two points in the hull
    // and the last three points do not make a right turn
    while (!isRightTurn(h1, h2, h3) && hull.size() > 2) {

      // remove the middle of the last three points
      hull.erase(hull.end() - 2);

      if (hull.size() >= 3) {
        h1 = hull.at(hull.size()-3);
      }
      h2 = hull.at(hull.size()-2);
      h3 = hull.at(hull.size()-1);
    }

    // going through left-to-right calculates the top hull
    // when we get to the end, we reverse direction
    // and go back again right-to-left to calculate the bottom hull
    if (currentPoint == points.size() -1 || currentPoint == 0) {
      direction = direction * -1;
    }

    currentPoint+= direction;

    if (hull.front()==hull.back()) break;
  }

  return ofPolyline(hull);
}

bool ofxLoadImage(ofImage &img, string filename) {
  if (!ofxFileExists(filename)) {
    ofxExit("ofxLoadImage: File not found: " + filename);
	return false; 
  } else {
    img.loadImage(filename);
	return true; 
  }
}

void ofxAssertFileExists(string filename, string msg) {
  ofFile file(filename);
  ofxAssert(ofxFileExists(filename), msg + ": File not found: " + filename);
}

void ofxTranslate(ofMesh &mesh, ofVec3f pos) {
  for (int i=0; i<mesh.getNumVertices(); i++) {
    mesh.getVertices()[i] += pos;
  }
}

void ofxTranslate(ofMesh &mesh, float x, float y, float z) {
  for (int i=0; i<mesh.getNumVertices(); i++) {
    mesh.getVertices()[i] += ofVec3f(x,y,z);
  }
}

void ofxRotate(ofMesh &mesh, float angle, ofVec3f axis) {
  for (int i=0; i<mesh.getNumVertices(); i++) {
    mesh.getVertices()[i].rotate(angle, axis);
  }
}

void ofxScale(ofMesh &mesh, float x, float y, float z) {
  for (int i=0; i<mesh.getNumVertices(); i++) {
    mesh.getVertices()[i].x *= x;
    mesh.getVertices()[i].y *= y;
    mesh.getVertices()[i].z *= z;
  }
}

ofColor ofxToColor(string s) {
  ofColor c;
  if (ofxStringStartsWith(s,"#")) ofStringReplace(s,"#","0x"); //#123456
  if (ofxStringStartsWith(s,"0x")) return ofColor::fromHex(ofxToInteger(s)); //0x123456 (hex)
  else if (ofStringTimesInString(s,",")==3) return ofxToColor(ofxToVec4f(s)); //255,255,255,128 (r,g,b,alpha)
  else if (ofStringTimesInString(s,",")==2) return ofxToColor(ofxToVec3f(s)); //255,255,255 (r,g,b)
  else if (ofStringTimesInString(s,",")==1) return ofColor(ofxToVec2f(s).x,ofxToVec2f(s).y); //255,128 (gray,alpha)
  else if (ofStringTimesInString(s,",")==0) return ofColor(ofxToInteger(s)); //gray
  else {
    ofLogError() << "ofxToColor(" << s << ") is not a valid color";
    return ofColor();
  }
}

void ofxRemoveWindowBorders() {
  #ifdef WIN32
  HWND m_hWnd = WindowFromDC(wglGetCurrentDC());
  LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
  style &= ~WS_DLGFRAME;
  style &= ~WS_CAPTION;
  style &= ~WS_BORDER;
  style &= WS_POPUP;
  LONG exstyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
  exstyle &= ~WS_EX_DLGMODALFRAME;
  ::SetWindowLong(m_hWnd, GWL_STYLE, style);
  ::SetWindowLong(m_hWnd, GWL_EXSTYLE, exstyle);
  SetWindowPos(m_hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
  #endif
}
