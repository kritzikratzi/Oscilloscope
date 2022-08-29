Oscilloscope
===

A software oscilloscope attempting to mimic the aesthetic of old school ray oscilloscopes. 
	
<img src="docs/screenshot.png" width="667">

# How to use

After starting the oscilloscope a welcome message is already loaded, press ▶︎ play to make sure your audio setup works. If it doesn't, open ⚙ settings and try to choose another output device. 

To open files you can either click the folder icon, or drag a file from Explorer/Finder/Nautilus/... to the application. 


|Key   |Action|
|------|------|
|Space |Play/Pause|
|f     |Fullscreen|
|Tab   |Hide interface|
|e     |Export to image sequence|



For more information

* either visit the user page at <a href="https://oscilloscopemusic.com/osci.php">oscilloscopemusic.com/osci.php</a>
* [download the latest version](https://github.com/kritzikratzi/Oscilloscope/releases)


# Programmer Documentation

## Project setup 

Things are moving around a bit sometimes. 
Make sure to read these instructions before each build. 

1. Download and unzip Openframeworks 0.10.1
1. Open the folder apps/myApps/ (in the OpenFrameworks folder)
1. Clone the repository with submodules: `git clone --recursive https://github.com/kritzikratzi/Oscilloscope.git`
1. Download the binary release for ofxAvCodec from [https://github.com/kritzikratzi/ofxAvCodec/releases/tag/0.2](https://github.com/kritzikratzi/ofxAvCodec/releases/tag/0.2)
1. Replace the addons/ofxAvCodec/libs with the libs folder from the download


### Compiling with XCode (OSX11.5+)

1. Add the `--deep` value to the `Other Code Signing Flags` key in the Build Settings tab
1. Open apps/myApps/oscilloscope/Oscilloscope.xcodeproject and then Build&Run
1. (Optional) Sign&Notarize with apple: Copy `scripts/osx-config-template.sh` to `osx-config.sh`, then run `scripts/dist.sh osx 1.1.0`


You can also build with `make && make run` on the command line. The resulting build will have no icon/no proper name/no retina support. 

### Compiling with Visual studio 2017

The shell commands can be run from a git bash, cygwin, msys, or any other shell emulator. 

1. run `scripts/clean.sh`
1. run `scripts/prepare.sh win64`
1. Open oscilloscope.sln and Build&Run

### Compiling with make in Linux

NOTE: Currently broken due to issues in the dependency ofxNative and ofxLiblaserdock also by Kritzikratzi. Workarounds and details in the next section.

Instructions for a full Ubuntu install including OpenFrameworks (thx rrolison68 and DJ_Level_3!)

1. Download the latest release of [https://github.com/openframeworks/openFrameworks/releases/latest](OpenFrameworks)
2. Extract it somewhere (you'll be doing everything inside this OpenFrameworks folder)
3. Open a terminal inside the OpenFrameworks folder or cd into it.
	Note: Make sure that this is the folder with the OpenFrameworks README and not a contanining folder!
		
Everything from here on out is done in this terminal, just run each command in order.
	
	sudo apt-get install gobjc++
	cd scripts/linux/ubuntu
	sudo ./install_dependencies.sh
	cd
	cd ../../../apps/myApps
	git clone --recursive https://github.com/kritzikratzi/Oscilloscope.git
	cd Oscilloscope
	scripts/clean.sh
	cp -R addons/ofxMightyUI/bin/data/* bin/data/
	cp -R addons/ofxFontAwesome/bin/data/* bin/data/
	make
	bin/Oscilloscope
See scripts/readme.md for the full distribution process. 

### Known Ubuntu compiler errors and workarounds

If you get the compiler error "Cocoa/Cocoa.h not found" or "Frameworks/Frameworks.h not found", ofxNative is broken. Comment out everything in addons/ofxNative/src/ofxNative_osx.mm. This is an addon by Kritzikratzi that adds commands for OSX, but it has some broken dependencies when running on Ubuntu. We're not using it since we're not on OSX, so commenting everything out works for now. An issue is open about this as of 8/29/2022.

If youo get a few compiler errors in a row, one of which mentions importing <cstdio>, ofxLiblaserdock is broken. Add `#include <cstdio>` to addons/ofxLiblaserdock/src/LaserdockDeviceManager.h, immediately below `#include <vector>`. A pull request is pending to fix this as of 8/29/2022.

### Package the software

* for osx run `scripts/dist.sh $platform $version`
* platform is one of `osx linux linux64 win64`
* version is whatever version you want, e.g. `1.0.6`

### Contributors

* [https://github.com/subwolf/](subwolf) Linux support
* [https://github.com/s-ol/](s-ol) Linux support
* [https://github.com/DJLevel3](DJ_Level_3) Fixing Linux Support


## License/Source code

* [Openframeworks](http://openframeworks.cc). A creative coding library. The larger part of it is licensed as MIT/BSD. 
* [FFmpeg](http://www.ffmpeg.org/) and [ofxAvCodec](https://github.com/kritzikratzi/ofxAvCodec). FFmpeg is _the_ encoder/decoder library and licensed under the gpl/lgpl 2.1. The binaries included here were compiled to comply with the lgpl. A copy of the LGPL together with instructions how the library was compiled for each platform can be found in the `legal` folder. 
* The sourcecode for this application is freely available on [github](https://github.com/kritzikratzi/Oscilloscope). 
