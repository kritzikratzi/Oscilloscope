#Oscilloscope - Packaging notes

===

These are my own notes how to bundle up the app for each platform so that i don't forget. 

# OSX


1. Open apps/myApps/oscilloscope/Oscilloscope.xcodeproject
1. Select the binary target and update the version info
1. Delete oscilloscope.app in finder, to make sure you have a fresh build
1. Build&Run
1. Code sign: `scripts/sign-mac.sh bin/Oscilloscope.app "Developer ID Application: Name of your certificate"` (run `security find-identity -v -p codesigning` to get a list of installed certs)
1. Create a zip in finder (don't use terminal, that creates a different zip i think)

You can also build with `make && make run` on the command line. The resulting build will have no icon/no proper name/no retina support. 

First patch OF to get rid of glut and fmodex: https://forum.openframeworks.cc/t/bundling-osx-app-without-glut-and-without-fmodex-nasty/23503

## Windows (Visual studio 2015)

1. run `scripts/clean.sh`
1. run `scripts/prepare.sh win32`
1. Open oscilloscope.sln
1. Open icon.rc, update the version info section
1. build
1. Run scripts/dist.sh win32 1.0.x

At this point audio should play through Asio (first choice), or Wasapi. You can follow these instructions to patch OF so you can pick at startup: http://pastebin.com/ZZLZ3jUm


### VS Project file 

#### Delay Load DLLs 

Configuration: Win32 / Release
Go to `Linker>All Options>Delay Load Dlls` and enter: 


	assimp.dll
	glut32.dll
	libeay32.dll
	ssleay32.dll
	swscale-3.dll
	Zlib.dll
	FreeType.dll
	fmodex.dll
	fmodexL.dll

None of these are actually used, ever. Specifying them as delay loaded allows you to just delete them. 



### Compiling with make in Linux

1. run `scripts/clean.sh`
1. run `scripts/prepare.sh linux64`
1. run `make && make run` ? [subwolf knows, i haven't done this in ages!]

See scripts/readme.md for the full distribution process. 

### Package the software

* for osx run `scripts/dist.sh $platform $version`
* platform is one of `osx linux linux64 win32 win64`
* version is whatever version you want, e.g. `1.0.6`


Helper Scripts
===


These scripts help manage the build process. 
You'll need a bash shell to run the scripts. 

The scripts should all figure out the correct paths themselves. There's no need to worry about the working directory. 


clean.sh
---

Cleans out the bin/ folder and removes files not in the repository. 
Changed files will not be reset. 

prepare.sh
---
Prepares the bin folder for a platform. This also fixes some problems with the build process that (i believe) I'm currently too dumb to fix directly in the make files. 

The script requires one argument, the operating system. Possible options are: 

* osx
* linux
* linux64
* win32
* win64

Depending on the parameter some dll files and other resources are copied to the bin folder. 
Use `clean.sh` to get rid of the mess again. 


dist.sh
---
Assembles a zip file ready for binary distribution. This adds the ffmpeg license files, compilation instructions, and does a whole lot of other things. It takes two parameters, one for the version, the other for the platform. 

A typical distribution workflow looks like this: 

	cd apps/myApps/Oscilloscope
	
	platform=osx
	version=1.0.3
	
	scripts/clean.sh
	scripts/prepare.sh $platform
	
	# now build & test (use xcode on mac, visual studio on windows, run make release on linux)
	
	scripts/dist.sh $platform $version
	
	
	
# OSX: Getting rid of GLUT and Fmodex (optional)

Those two libraries aren't needed. Removing them is done all at once, like this: 


Remove the setting `FRAMEWORK_SEARCH_PATHS_QUOTED_FOR_TARGET_1 = "\"$(SRCROOT)/../../../libs/glut/lib/osx\""` (you can find it through xcode cmd+shift+f search)

Then, in `CoreOF.xcconfig`, redefine LIB_FMODEX and change LIB_FMODEX: 

    // disable fmodex!
    LIB_FMODEX = -Wl,-U,_FMOD_System_Close
    REMOVED_DEPS = yes

    OF_CORE_FRAMEWORKS = -framework Accelerate -framework AGL -framework AppKit -framework ApplicationServices -framework AudioToolbox -framework AVFoundation -framework Cocoa -framework CoreAudio -framework CoreFoundation -framework CoreMedia -framework CoreServices -framework CoreVideo -framework IOKit -framework OpenGL -framework QuartzCore -framework QuickTime -framework QTKit //-framework GLUT


Now go into the project config, build phases, run scripts, and wrap the glut and fmodex entries in an if: 
	
	if [ REMOVED_DEPS -ne "yes"]
	then
		# Copy libfmod and change install directory for fmod to run
		rsync -aved ../../../libs/fmodex/lib/osx/libfmodex.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/";
		install_name_tool -change @executable_path/libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";
		# Copy GLUT framework (must remove for AppStore submissions)
		rsync -aved ../../../libs/glut/lib/osx/GLUT.framework "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/"
	fi

This way the build should still work if someone hasn't tweaked their OF installation. 