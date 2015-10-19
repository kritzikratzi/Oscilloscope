#include "ofMain.h"
#include "ofGLProgrammableRenderer.h"
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
	ofGLWindowSettings settings;
	settings.setGLVersion(3,2);
	settings.width = 1400;
	settings.height = 768;
	ofCreateWindow(settings);
	
/*	#if defined(TARGET_OSX)
	NSWindow * cocoaWindow = (NSWindow*)window.getCocoaWindow();
	[cocoaWindow setFrame:[[NSScreen mainScreen] visibleFrame] display:YES];
	#endif
*/
	
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
