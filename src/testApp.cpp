#include "testApp.h"
#include "util.h"
#include <Poco/Mutex.h>
#include "ofxIniSettings.h"

Poco::Mutex mutex;
Poco::Mutex updateMutex;

bool applicationRunning = false;


//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	ofBackground(0);
	ofSetBackgroundAuto(false);
	dotImage.allocate(64, 64, OF_IMAGE_COLOR_ALPHA);
	dotImage.loadImage( "dot.png" );
	ofEnableSmoothing(); 
	
	cout << "Available Sound Devices: " << endl;
	soundStream.listDevices();
	cout << "=============================" << endl << endl;
	
	
	vector<RtAudio::DeviceInfo> devices = listRtSoundDevices();
	ofSetFrameRate(60);
	
	root = new mui::Root();
	
	settings.loadFromFile();
	
	configView = new ConfigView();
	configView->fromSettings( settings );
	root->add( configView );
	
	meshView = new MeshView();
	meshView->visible = false;
	root->add( meshView );
	
	left1.loop = false;
	right1.loop = false;
	left2.loop = false;
	right2.loop = false;
	
	TCP = NULL;
	
	fetcher = new Fetcher( this );
	fetcher->startThread();

	lx1 = lx2 = ly1 = ly2 = 0;
	c1 = ofColor( 0 );
	c2 = ofColor( 0 );
	
	s1 = 0;
	s2 = 0;
}


void testApp::startApplication(){
	if( applicationRunning ) return;
	applicationRunning = true;
	
	left1.play();
	right1.play();
	left2.play();
	right2.play();

	configView->toSettings( settings );
	meshView->flipXY->selected = settings.flipXY;
	meshView->invertX->selected = settings.invertX;
	meshView->invertY->selected = settings.invertY;
	meshView->scaleSlider->slider->value = settings.scale;
	meshView->lineWidth->slider->value = settings.lineWidth;
	meshView->clearBg->slider->value = settings.clearBg;
	meshView->interpolationSteps->slider->value = settings.interpolationSteps;
	meshView->alpha->slider->value = settings.alpha;
	meshView->beta->slider->value = settings.beta;

	settings.saveToFile();
	configView->visible = false;
	meshView->visible = true;
	
	//if you want to set the device id to be different than the default
	cout << "Opening Sound Card: " << endl;
	cout << "    Sample rate: " << settings.sampleRate << endl;
	cout << "    Buffer size: " << settings.bufferSize << endl;
	cout << "    Num Buffers: " << settings.numBuffers << endl;
	soundStream.setDeviceID( settings.deviceId );
	soundStream.setup(this, 0, 2, settings.sampleRate, settings.bufferSize, settings.numBuffers);
	
	TCP = new ofxTCPServer();
	TCP->setup(settings.port,true);
}


void testApp::stopApplication(){
	configView->fromSettings(settings);
	settings.flipXY = meshView->flipXY->selected;
	settings.invertX = meshView->invertX->selected;
	settings.invertY = meshView->invertY->selected;
	settings.scale = meshView->scaleSlider->slider->value;
	settings.lineWidth = meshView->lineWidth->slider->value;
	settings.clearBg = meshView->clearBg->slider->value;
	settings.interpolationSteps = meshView->interpolationSteps->slider->value;
	settings.alpha = meshView->alpha->slider->value;
	settings.beta = meshView->beta->slider->value;
	settings.saveToFile();
	
	if( !applicationRunning ) return;
	applicationRunning = false;
	soundStream.stop();
	soundStream = ofSoundStream();
	configView->visible = true;
	meshView->visible = false;
	
	TCP->close();
	delete TCP;
}



//--------------------------------------------------------------
void testApp::update(){
	root->handleUpdate();
	if( !applicationRunning ) return;
	
	update( shapeMesh1, left1, right1, a1, b1, p1, c1, s1, 0 );
	update( shapeMesh2, left2, right2, a2, b2, p2, c2, s2, 1 );
}

void testApp::update( ofMesh &shapeMesh, MonoSample &left, MonoSample &right, ofPoint &a, ofPoint &b, ofPoint &p, ofColor &lastColor, float &s, int index ){
	shapeMesh.clear();
	shapeMesh.setMode(OF_PRIMITIVE_LINES);
	shapeMesh.enableColors();
	
	static float * leftBuffer = new float[512];
	static float * rightBuffer = new float[512];
	bool didThing = false; 

	float S= ofGetWidth()/2*meshView->scaleSlider->slider->value;
	if( left.totalLength >= 512 && right.totalLength >= 512 ){

		while( left.totalLength >= 512 && right.totalLength >= 512 ){
			didThing = true; 
			memset(leftBuffer,0,512*sizeof(float));
			memset(rightBuffer,0,512*sizeof(float));
			
			left.addTo(leftBuffer, 1, 512);
			right.addTo(rightBuffer, 1, 512);
			left.peel(512);
			right.peel(512);
			
	//		float x2 = lx;
	//		float y2 = ly;
			float dMax = 0;
			if( shapeMesh.getVertices().size() < 1024*8 ){
				for( int i = 0; i < 512-1; i++ ){
					ofPoint c = ofPoint( leftBuffer[i], rightBuffer[i] );
					ofPoint d = ofPoint( leftBuffer[i+1], rightBuffer[i+1] );
					float dist = (b-c).length();
					s = dist;
					float alph = powf(ofClamp( 1-meshView->alpha->slider->value*s, 0, 1),meshView->beta->slider->value);
					float E = powf(ofClamp( 1-40*s, 0, 1),40);
					float alpha = 255*MAX(E,alph);
					float erosion = 150*E;
					lastColor = ofColor(index==0?255:erosion, index==1?255:erosion, erosion, alpha);
					
//					float ab = 255*powf(ofClamp( 1-alpha*s, 0, 1),meshView->beta->slider->value);
					
					dMax = MAX(dist, dMax);
					//shapeMesh.addVertex(b*S);
					float N = ofClamp((int)meshView->interpolationSteps->slider->value,1,100);
					CMRPoly poly = catmullRomPoly( a, b, c, d );
					for( int j = 0; j <= N; j++ ){
						ofPoint pTilde = poly.eval(j/N );
						//float d = ofDist(x1, y1, lx, ly);
						//lastColor = ofColor(0, 255, 0, 255*powf(ofClamp(1-powf(dist,ofGetMouseY()*0.0001),0,1),ofGetMouseX()*0.001));

						shapeMesh.addVertex(p*S);
						shapeMesh.addColor(lastColor);
						p = pTilde;
						shapeMesh.addVertex(p*S);
						shapeMesh.addColor(lastColor);
					}
					//shapeMesh.addVertex(c);
					a = b;
					b = c;
				}
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	testMesh.draw();

	ofEnableAlphaBlending();
	ofSetColor( 0, meshView->clearBg->slider->value );
	ofFill();
	ofRect( 0, 0, ofGetWidth(), ofGetHeight() );
	
	ofShowCursor();
	ofPushMatrix();
	ofTranslate(ofGetWidth()/2, ofGetHeight()/2 );
	
	int scaleX = 1;
	int scaleY = -1;
	if( meshView->invertX->selected ) scaleX *= -1;
	if( meshView->invertY->selected ) scaleY *= -1;
	
	
	ofScale( scaleX, scaleY );
	if( meshView->flipXY->selected ){
		ofRotate(-90);
		ofScale( -1, 1 );
	}
	
	ofSetColor(255, 0, 0, 25);
	ofLine( -10, 0, 10, 0 );
	ofLine( 0, -10, 0, 10 );

	draw( shapeMesh1, 0 );
	draw( shapeMesh2, 1 );
/*	vector<ofVec3f> verts = shapeMesh.getVertices();
	vector<ofVec3f>::iterator it = verts.begin();
	ofSetColor(255,20);
	int i = 0;
	while( it != verts.end() ){
		if( i % 10 == 0 )
			dotImage.draw((*it).x-dotImage.width/2, (*it).y-dotImage.height/2);
		++i;
		++it;
	}*/
//	ofDisableBlendMode();
	
	ofPopMatrix();
	
	root->handleDraw();
	if( meshView->visible ){
		ofSetColor(255);
		ofDrawBitmapString("+C=" + ofToString(left1.totalLength) + "samples", 500,10);
		ofDrawBitmapString("+R=" + ofToString(left2.totalLength) + "samples", 500,30);
	}

}

inline void setColor( int c, int alpha, int index ){
	if( index == 0 ) ofSetColor( 255, c, c, alpha ); 
	else ofSetColor( c, 255, c, alpha ); 
}

inline void setColors( vector<ofFloatColor> &colors, int igrey, int ialpha, int index ){
	float grey = igrey/255.0;
	float alpha = ialpha/255.0;
	
	vector<ofFloatColor>::iterator it = colors.begin();
	while( it != colors.end() ){
		(*it).a = alpha;
		(*it).b = grey;
		if( index == 0 ) (*it).g = grey;
		else (*it).b = grey;
		
		++it;
	}
}

void testApp::draw( ofMesh &shapeMesh, int index ){
/*	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	float s = meshView->lineWidth->slider->value;
//	setColors( shapeMesh.getColors(), 50, 30, index );
	ofSetLineWidth(20.0*s);
	shapeMesh.draw();
	
//	setColors( shapeMesh.getColors(), 50, 50, index );
	ofSetLineWidth(5.0*s);
	shapeMesh.draw();
	
//	setColors( shapeMesh.getColors(), 75, 50, index );
	ofSetLineWidth(2.5*s);
	shapeMesh.draw();
	
/	setColors( shapeMesh.getColors(), 0, 255, index  );*/
	float s = meshView->lineWidth->slider->value;
	ofSetLineWidth(s);
	shapeMesh.draw();
}

void testApp::exit(){
	stopApplication();
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	if( root->handleKeyPressed( key ) ) return;
	
	if( key == '\t' && !configView->isVisibleOnScreen()){
		meshView->visible = !meshView->visible;
	}

	if( key == 'f' ){
		ofToggleFullscreen(); 
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
	if( root->handleKeyReleased( key ) ) return;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	if( root->handleMouseMoved( x, y ) ) return;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	if( root->handleMouseDragged( x, y, button ) ) return;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	if( root->handleMousePressed( x, y, button ) ) return;
}


//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	if( root->handleMouseReleased( x, y, button ) ) return;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){
	if( left1.totalLength < 1024*8 ){
		left1.append(input+0, bufferSize,nChannels);
		right1.append(input+1,bufferSize,nChannels);
	}
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
	if( msg.message == "start-pressed" ){
		startApplication();
	}
	else if( msg.message == "stop-pressed" ){
		stopApplication();
	}
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}


Fetcher::Fetcher( testApp * app ){
	this->app = app;
}

void Fetcher::threadedFunction(){
	while( isThreadRunning() ){
		ofxTCPServer * TCP = app->TCP;
		if( TCP != NULL && TCP->isConnected() ){
			for( int x = 0; x < 50; x ++ ){
				for(int i = 0; i < TCP->getLastID(); i++){
					if( !TCP->isClientConnected(i) ) continue;
					//string port = ofToString( TCP.getClientPort(i) );
					string ip   = TCP->getClientIP(i);
					static float * result = NULL;
					int len = app->settings.bufferSize*2*sizeof(float);
					if( result == NULL ){
						result = new float[192000*2];
					}
					int received = TCP->receiveRawBytes(i, (char*)result, app->settings.bufferSize*2*sizeof(float) );
					if( app->left2.totalLength < 1024*8 ){
						app->left2.append(result+0, app->settings.bufferSize,2);
						app->right2.append(result+1,app->settings.bufferSize,2);
					}
				}
			}
		}
		ofSleepMillis(1);
	}
}
