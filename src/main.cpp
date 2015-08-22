#include "ofMain.h"
#include "ofApp.h"
#include "sounddevices.h"
#include "MUI.h" 
#if defined(TARGET_OSX)
#import <AppKit/AppKit.h>
#endif

#if defined(__linux__)
#include "ofAppGlutWindow.h"
#else
#include "ofAppGLFWWindow.h"
#endif

#include <GL/glew.h>

//========================================================================
int main( ){
//	ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	// using glut on linux, because http://forum.openframeworks.cc/t/application-wont-start-after-ubuntu-upgrade/17491
	#if defined(__linux__)
	ofAppGlutWindow window;
	#else
	ofAppGLFWWindow window;
	#endif
	cout << "LAUNCH DESKTOP" << endl;
	ofSetupOpenGL(&window, 1000,1000, OF_WINDOW);


	if(!ofGLCheckExtension("GL_ARB_geometry_shader4") &&
	   !ofGLCheckExtension("GL_EXT_geometry_shader4") &&
	   !ofIsGLProgrammableRenderer()){
		ofLogFatalError() << "geometry shaders not supported on this graphics card";
		return 1;
	}


	#if defined(TARGET_OSX)
	NSWindow * cocoaWindow = (NSWindow*)window.getCocoaWindow();
	[cocoaWindow setFrame:[[NSScreen mainScreen] visibleFrame] display:YES];
//	[cocoaWindow setLevel:10];
	#endif

	mui_init();
	mui::MuiConfig::font = "mui/fonts/Lato-Regular.ttf";
	ofRunApp(new ofApp);
	
}

#ifdef _WIN32
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    return main();
}
#endif
