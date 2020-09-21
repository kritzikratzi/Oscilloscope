#include "ofMain.h"
#include "ofApp.h"
#include "ofxMightyUI.h"

#if defined(TARGET_OSX)
#import <AppKit/AppKit.h>
#elif defined(TARGET_WIN32)
#include "../resource.h"
#endif
#include "version.h"

#include "ofAppGLFWWindow.h"
#include "ofxNative.h"
#include <GL/glew.h>

//========================================================================
int main(){
	// please read about the asio/wasapi patch for of0.9x in windows: http://pastebin.com/ZZLZ3jUm

	globals.loadFromFile();
	ofSetEscapeQuitsApp(false);

	ofAppGLFWWindow window;

	cout << "LAUNCH DESKTOP" << endl;
	ofSetupOpenGL(&window, 1024, 700, OF_WINDOW);
	window.setWindowTitle("Oscilloscope " + app_version);
	
	// go fullscreen in mac osx
	ofxNative::maximizeWindow(window);
	
	#if defined(TARGET_OSX)
	NSWindow * cocoaWindow = (NSWindow*)window.getCocoaWindow();
//	[cocoaWindow setFrame:[[NSScreen mainScreen] visibleFrame] display:YES];
	NSString * ver = [NSString stringWithUTF8String:app_version.c_str()];
	NSString * title = [NSString stringWithFormat:@"Oscilloscope %@", ver];
	[cocoaWindow setTitle:title];
	if(globals.alwaysOnTop){
		[cocoaWindow setLevel: NSFloatingWindowLevel];
	}
	#endif

	#if defined(TARGET_WIN32)
	HWND hwnd = ofGetWin32Window();
	HICON hMyIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(MAIN_ICON));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hMyIcon);
	#endif

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


void setWindowRepresentedFilename( string filename ){
#if defined(TARGET_OSX)
	NSWindow * cocoaWindow = (NSWindow*)(ofGetWindowPtr()->getCocoaWindow());
	if(filename == ""){
		[cocoaWindow setRepresentedFilename:@""];
		NSString * ver = [NSString stringWithUTF8String:app_version.c_str()];
		NSString * title = [NSString stringWithFormat:@"Oscilloscope %@", ver];
		[cocoaWindow setTitle:title];
	}
	else if(filename[0] != '/'){
		[cocoaWindow setRepresentedFilename:@""];
		[cocoaWindow setTitle:[NSString stringWithUTF8String:filename.c_str()]];
	}
	else{
		NSString * file = [NSString stringWithUTF8String:filename.c_str()];
		NSString * filename = [file pathComponents].lastObject;
		[cocoaWindow setTitle:filename];
		[cocoaWindow setRepresentedFilename:file];
	}
#else
	if(filename==""){
		ofGetWindowPtr()->setWindowTitle("Oscilloscope " + " " + app_version);
	}
	else{
		ofGetWindowPtr()->setWindowTitle(filename);
	}
#endif
	
}


// returns a path where the application can only read.
// for osx this is the "resources" folder residing inside the application bundle,
// for windows it is still the data folder.
// if the file does not exist in the application bundle, the data folder is returned.
string ofxToReadonlyDataPath( string filename ){
#ifdef TARGET_OSX
	// http://www.cocoabuilder.com/archive/cocoa/193451-finding-out-executable-location-from-c-program.html
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFURLRef    url  = CFBundleCopyResourcesDirectoryURL(bundle);
	CFURLRef absolute = CFURLCopyAbsoluteURL(url);
	Boolean abs;
	CFStringRef path  = CFURLCopyFileSystemPath(absolute,kCFURLPOSIXPathStyle);
	CFIndex    maxLength = CFStringGetMaximumSizeOfFileSystemRepresentation(path);
	char        *result = (char*)malloc(maxLength);
	
	if(result) {
		if(!CFStringGetFileSystemRepresentation(path,result, maxLength)) {
			free(result);
			result = NULL;
		}
	}
	
	string realResult = string([[NSString stringWithUTF8String:result] stringByAppendingPathComponent:[NSString stringWithUTF8String:filename.c_str()]].UTF8String );
	
	CFRelease(path);
	CFRelease(url);
	CFRelease(absolute);
	
	if( ofFile(realResult, ofFile::Reference).exists() ){
		return realResult;
	}
	else{
		return ofToDataPath(filename,true); 
	}
#else
	return ofToDataPath(filename,true);
#endif
}


// returns a path where the application can safely read+write data.
// this is great for settings files and the like.
// for osx this is the application support directory (without sandboxing)
// or ~/Library/Containers/<bundle-identifier>/
// for windows this is in %APPDATA%
string ofxToReadWriteableDataPath( string filename ){
	return ofxNative::getSystemDataFolder() + "/" + filename;
}
