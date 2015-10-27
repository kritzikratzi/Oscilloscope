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
	
	MonoSample test;
	float val = 0;
	for( int i = 2; i < 20; i++ ){
		float data[i];
		for( int j = 0; j < i; j++ ){
			data[j] = val;
			val += 0.1;
		}
		test.append(data, i);
	}
	
	// take out 1 at a time.
	test.play();
	
	while( test.totalLength > 0 ){
		for( int i = 1; i < test.totalLength%2+1; i++ ){
			float data[i];
			for( int j = 0; j < i; j++ ){
				data[j] = val;
				val += 0.1;
			}
			test.append(data, i);
		}
		

		test.play();
		int len = test.totalLength%8+3;
		float * res = new float[len];
		memset(res,0,len*sizeof(float));
		len = test.addTo(res, 1, len);
		test.peel(len);
		for( int i = 0; i < len; i++ ){
			cout << i << ": " << res[i] << endl;
		}
		
		delete res;
	}
	
	ofSetEscapeQuitsApp(false);

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
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	
	// go fullscreen in mac osx
	#if defined(TARGET_OSX)
	NSWindow * cocoaWindow = (NSWindow*)window.getCocoaWindow();
	[cocoaWindow setFrame:[[NSScreen mainScreen] visibleFrame] display:YES];
	#endif
	


	if(!ofGLCheckExtension("GL_ARB_geometry_shader4") &&
	   !ofGLCheckExtension("GL_EXT_geometry_shader4") &&
	   !ofIsGLProgrammableRenderer()){
		ofSystemAlertDialog("Geometry shader extension (GL_EXT_geometry_shader4) not available :(");
		ofLogFatalError() << "geometry shaders not supported on this graphics card" << endl;
		return 1;
	}


	mui_init();
	mui::MuiConfig::font = "mui/fonts/Lato-Regular.ttf";
	ofRunApp(new ofApp);
	
}

#ifdef _WIN32
// handle windows startup
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    return main();
}
#endif
