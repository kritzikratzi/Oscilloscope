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