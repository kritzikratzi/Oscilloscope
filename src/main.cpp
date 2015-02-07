#include "ofMain.h"
#include "testApp.h"
#include "sounddevices.h"
#include "MUI.h" 
#include "ofAppGLFWWindow.h"
#import <AppKit/AppKit.h>

//========================================================================
int main( ){

//	ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofAppGLFWWindow window;
	cout << "LAUNCH DESKTOP" << endl;
	ofSetupOpenGL(&window, 1000,1500, OF_WINDOW);

	#if defined(TARGET_OSX)
	NSWindow * cocoaWindow = (NSWindow*)window.getCocoaWindow();
//	[cocoaWindow setLevel:10];
	#endif

	mui_init();
	mui::MuiConfig::font = "mui/fonts/Lato-Regular.ttf";
	ofRunApp(new testApp);
	
}
