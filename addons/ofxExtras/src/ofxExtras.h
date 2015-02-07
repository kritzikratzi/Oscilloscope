#pragma once

#include "ofAppGlutWindow.h"
#include "ofMain.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Path.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/URIStreamOpener.h"

#define ofxBeginApp() class ofApp : public ofBaseApp { public:
#define ofxEndApp() }; int main() { ofSetupOpenGL(new ofAppGlutWindow,500,500,OF_WINDOW); ofRunApp(new ofApp); }
#define ofxPrintln(str) cout << str << endl;
#define foreach(t,v) for(typeof(v.begin()) p=v.begin(); p!=v.end(); p++) { typeof(*p) &t=*p;
#define endfor }
#define TAU TWO_PI

class ofxLatLon {
public:
  ofxLatLon() : lat(0), lon(0) {}
  ofxLatLon(float _lat, float _lon) { set(lat,lon); }
  void set(float _lat, float _lon) { lat = _lat; lon = _lon; }
  float lat,lon;
};

bool ofxContains(vector<string> strings, string key);
bool ofxColorMatch(ofColor a, ofColor b, int tolerance=0);
bool ofxFileExists(string filename);
bool ofxIsWindows();
bool ofxMouseMoved(); //this one should be fixed to update prev only once per update/draw not per call
bool ofxOnTimeIntervalSeconds(int s);
bool ofxStringEndsWith(string str, string key);
bool ofxStringStartsWith(string str, string key);
bool ofxToBoolean(float f);
bool ofxToBoolean(string sValue);
//enum ofxAlign { LEFT, CENTER, RIGHT };
float ofxDist(float x1, float y1, float z1, float x2, float y2, float z2);
float ofxGetHeading(ofPoint p, ofPoint anchor=ofPoint(0,0)); //radians
float ofxLerp(float start, float end, float amt);
int ofxGetMultiByteStringLength(string s); //returns the actual string length instead of the number of chars/bytes
int ofxIndex(float x, float y, float w);
int ofxToInteger(ofColor c);
int ofxToInteger(string str);
ofColor ofxToColor(ofVec4f v);
ofColor ofxToColor(ofVec3f v, int alpha=255);
ofColor ofxToColor(int hexColor);
ofColor ofxToColor(string s); //parse a color from a string
ofColor ofxToColor(unsigned char r, unsigned char g, unsigned char b);
ofMatrix4x4 ofxToMatrix4x4(string str);
ofMesh ofxCreateGeoSphere(int stacks=32, int slices=32);
ofPoint ofxGetMouse();
ofPoint ofxGetPreviousMouse();
ofPoint ofxGetCenter();
ofPoint ofxGetCenterOfMass(vector<ofPoint*> points);
ofPoint ofxGetMouseFromCenter();
ofPoint ofxGetPointOnCircle(float angle, float radius); //radians
ofPoint ofxLerp(ofPoint start, ofPoint end, float amt);
ofQuaternion ofxToQuaternion(float lat, float lon);
ofQuaternion ofxToQuaternion(ofxLatLon ll);
ofQuaternion ofxToQuaternion(string str);
ofRectangle ofxGetBoundingBox(vector<ofPoint*> points);
ofRectangle ofxGetBoundingBox(vector<ofPoint> &points);
ofRectangle ofxToRectangle(ofVec4f v);
ofRectangle ofxToRectangle(string str);
ofRectangle ofxScaleRectangle(ofRectangle rect, float s);
ofVec2f ofxToVec2f(string str);
ofVec3f ofxMouseToSphere(float x, float y); //-0.5 ... 0.5
ofVec3f ofxMouseToSphere(ofVec2f v); //-0.5 ... 0.5
ofVec3f ofxToCartesian(float lat, float lon);
ofVec3f ofxToCartesian(ofQuaternion q);
ofVec3f ofxToVec3f(float *a);
ofVec3f ofxToVec3f(string str);
ofVec4f ofxToVec4f(string str);
vector<float> ofxToFloatVector(string s, string delimiter=",");
ofxLatLon ofxToLatLon(ofQuaternion q);
ofxLatLon ofxToLatLon(string s);
string ofxAddTrailingSlash(string foldername);
string ofxAddTrailingString(string str, string trail);
string ofxFormatDateTime(time_t rawtime, string format);
string ofxFormatString(string format, int number);
string ofxFormatString(string format, string s);

#ifdef TARGET_OS_X
unsigned int ofxGetFileAge(string filename);
#endif

string ofxGetFileExtension(string filename);
string ofxGetFilenameFromUrl(string url);
string ofxGetHostName();
string ofxGetSerialString(ofSerial &serial, char until);
string ofxGetSerialString2(ofSerial &serial, char until);
string ofxReplaceString(string input, string replace, string by);

string ofxFormatDateTime(time_t rawtime, string format);
time_t ofxParseDateTime(string datetime, string format);
time_t ofxGetDateTime();
string ofxGetIsoDateTime();

//vector<string> ofxParseString(string str, string format);

///template<typename T> T ofxFromList(vector<T> &list, float normIndex);
template<typename T> T ofxFromList(vector<T> &list, float normIndex) {
    int index = ofClamp(normIndex * list.size(), 0,list.size()-1);
    if (list.size()==0) return T();
    return list[index];
}

bool ofxStringEndsWith(string str, string key);
bool ofxStringStartsWith(string str, string key);
void ofxScale(float scale);
void ofxNotice(string msg);
void ofxSetHexColor(int hexColor, int a=255);
void ofxSetColor(ofColor c);
void ofxSetColorHSB(int h, int s, int b, int a=255);
float ofxDist(float x1, float y1, float z1, float x2, float y2, float z2);
//bool ofxColorMatch(ofColor a, ofColor b, int tolerance=0);

string ofxStringAfterFirst(string str, string key);
string ofxStringAfterLast(string str, string key);
string ofxStringBeforeFirst(string str, string key);
string ofxStringBeforeLast(string str, string key);

string ofxToHexString(int value, int digits);
string ofxToString(bool value);
string ofxToString(char ch);
string ofxToString(float f);
string ofxToString(int value);
string ofxToString(ofColor c);
string ofxToString(ofMatrix4x4 m);
string ofxToString(ofQuaternion q);
string ofxToString(ofRectangle value);
string ofxToString(ofVec2f value);
string ofxToString(ofVec3f v, int precision=7);
string ofxToString(ofVec4f value);
string ofxToString(string str);
string ofxToString(unsigned char ch);
string ofxTrimString(string str);
string ofxTrimStringLeft(string str);
string ofxTrimStringRight(string str);
string ofxUrlToSafeLocalPath(string url);
string ofxWordWrap(string input, int maxWidth, ofTrueTypeFont *font=0);
time_t ofxGetDateTime();
time_t ofxParseDateTime(string datetime, string format);
vector<ofPoint*> ofxGetPointsFromPath(ofPath &path);
vector<string> ofxLoadStrings(string url);
vector<string> ofxReadFiles(string folder);
vector<string> ofxToStringVector(string value);

void ofxArc(float radius, float startAngle, float stopAngle, int detail=32); //radians
void ofxArcStrip(float innerRadius, float outerRadius, float startAngle, float stopAngle); //radians
void ofxAssert(bool condition, string message);
void ofxAutoColorMesh(ofMesh &mesh);
void ofxDisableDepthTest();
void ofxDisableTexture();
void ofxDrawDisk(ofBaseHasTexture &img,float r, float slices=24);
void ofxDrawSphere(float radius, int segments=32);
void ofxDrawVertex(ofVec3f v);
void ofxEnableDepthTest();
void ofxExit();
void ofxExit(string msg);
void ofxNotice(string msg);
void ofxQuadricDisk(float innerRadius, float outerRadius, int resolution=32);
void ofxQuadricSphere(float radius, int resolution=32);
void ofxQuadWarp(ofBaseHasTexture &tex, ofPoint lt, ofPoint rt, ofPoint rb, ofPoint lb, int rows=10, int cols=10);
void ofxResetTransform(ofNode &n);
void ofxRotate(float angle, ofVec3f v);
void ofxRotate(ofNode &node, ofQuaternion q);
void ofxRotate(ofQuaternion q);
void ofxRotate(ofVec3f v);
void ofxSaveString(string filename, string str);
void ofxSaveStrings(string filename, vector<string> lines);
void ofxScale(float scale);
void ofxScale(ofVec3f v);
//void ofxSerialWrite(ofSerial &serial, string str);
//void ofxSerialWriteLine(ofSerial &serial, string str);
void ofxSetColor(ofColor c);
//void ofxSetColorHSB(int h, int s, int b, int a=255);
void ofxSetCursor(bool bVisible);
//void ofxSetHexColor(int hexColor, int a=255);
void ofxSetTexture(ofBaseHasTexture &material);
void ofxSetTexture(ofTexture &texture);
void ofxSetWindowRect(ofRectangle w);
//void ofxSimplifyPath(ofPath &path, int iterations=10, float amount=15, float distance=1); //of008 has no subpaths
vector<ofPolyline> ofxGetPolylinesFromPath(ofPath path);
void ofxTranslate(ofVec3f v);
int ofxMakeEven(int v, int add=1);
int ofxMakeOdd(int v, int add=1); // you can choose -1 to subtract
ofPolyline ofxGetConvexHull(vector<ofPoint> points);
bool isRightTurn(ofPoint a, ofPoint b, ofPoint c);
bool lexicalComparison(const ofPoint& v1, const ofPoint& v2);
bool ofxLoadImage(ofImage &img, string filename);
void ofxAssertFileExists(string filename, string msg="");
void ofxTranslate(ofMesh &mesh, ofVec3f pos);
void ofxTranslate(ofMesh &mesh, float x, float y, float z=0);
void ofxRotate(ofMesh &mesh, float angle, ofVec3f axis);
void ofxScale(ofMesh &mesh, float x, float y, float z);
void ofxRemoveWindowBorders();
