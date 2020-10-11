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

This is a full install including OF (thx rrolison68!)

	cd
	wget -c https://openframeworks.cc/versions/v0.10.1/of_v0.10.1_linux64gcc6_release.tar.gz
	tar -zxvf of_v0.10.1_linux64gcc6_release.tar.gz
	mv of_v0.10.1_linux64gcc6_release OF
	cd OF/scripts/linux/ubuntu
	sudo ./install_dependencies.sh
	cd
	cd OF/apps/myApps
	git clone --recursive https://github.com/kritzikratzi/Oscilloscope.git
	cd Oscilloscope
	scripts/clean.sh
	cp -R addons/ofxMightyUI/bin/data/* bin/data/
	cp -R addons/ofxFontAwesome/bin/data/* bin/data/
	make -j2
	bin/Oscilloscope
See scripts/readme.md for the full distribution process. 

### Package the software

* for osx run `scripts/dist.sh $platform $version`
* platform is one of `osx linux linux64 win64`
* version is whatever version you want, e.g. `1.0.6`

### Contributors

* [https://github.com/subwolf/](subwolf) Linux support
* [https://github.com/s-ol/](s-ol) Linux support


## License/Source code

* [Openframeworks](http://openframeworks.cc). A creative coding library. The larger part of it is licensed as MIT/BSD. 
* [FFmpeg](http://www.ffmpeg.org/) and [ofxAvCodec](https://github.com/kritzikratzi/ofxAvCodec). FFmpeg is _the_ encoder/decoder library and licensed under the gpl/lgpl 2.1. The binaries included here were compiled to comply with the lgpl. A copy of the LGPL together with instructions how the library was compiled for each platform can be found in the `legal` folder. 
* The sourcecode for this application is freely available on [github](https://github.com/kritzikratzi/Oscilloscope). 
