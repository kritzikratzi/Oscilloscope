Oscilloscope
===

A software oscilloscope attempting to mimic the aesthetic of old school ray oscilloscopes. 
	
<img src="docs/screenshot.png" width="667">


## Not a programmer? 

This is the programming documentation. If you are not a programmer please

* either visit the user page at <a href="http://asdfg.me/osci/">asdfg.me/osci/</a>
* or click on the [releases tab](https://github.com/kritzikratzi/Oscilloscope/releases) and download the latest version for your platform



# Project setup 

1. Download and unzip Openframeworks 0.9.2 or newer
1. Move this project into apps/myApps/oscilloscope
1. Clone [ofxMightyUI](https://github.com/kritzikratzi/ofxMightyUI) into addons/ofxMightyUI. Switch to the branch `fs2`! 
1. Clone [ofxAvCodec](https://github.com/kritzikratzi/ofxAvCodec) into ofxAvCodec. 
1. (This is seriously painful) Follow the instructions in ofxAvCodec/ffmpeg_src/readme.md and compile shared libraries for your platform
1. Clone [ofxFontAwesome](https://github.com/kritzikratzi/ofxFontAwesome) into addons/ofxFontAwesome
1. Clone [ofxFontStash2](https://github.com/armadillu/ofxFontStash2) into addons/ofxFontStash2


### Compiling with XCode

1. run `scripts/clean.sh`
1. run `scripts/prepare.sh osx`
1. Open apps/myApps/oscilloscope/Oscilloscope.xcodeproject and then Build&Run

You can also build with `make && make run` on the command line. The resulting build will have no icon/no proper name/no retina support. 

### Compiling with Visual studio

**The visual studio project files are not up to date**

1. run `scripts/clean.sh`
1. run `scripts/prepare.sh win32`
1. Open oscilloscope.sln and Build&Run

### Compiling with make in Linux

1. run `scripts/clean.sh`
1. run `scripts/prepare.sh linux64`
1. run `make && make run` ? [subwolf knows, i haven't done this in ages!]

See scripts/readme.md for the full distribution process. 

### Package the software

* for osx run `scripts/dist.sh $platform $version`
* platform is one of `osx linux linux64 win32 win64`
* version is whatever version you want, e.g. `1.0.6`

## Experimenting with the shaders

Once you have a running version (either compiled or a binary release) you can manipulate the shaders on the fly. The shaders will be reloaded automatically when they're saved. Please note that you're bound to OpenGL2 / GLSL 1.2 (shader 120 with gl_ext_geometry_shader)


## License/Source code

* [Openframeworks](http://openframeworks.cc). A creative coding library. The larger part of it is licensed as MIT/BSD. 
* [FFmpeg](http://www.ffmpeg.org/) and [ofxAvCodec](https://github.com/kritzikratzi/ofxAvCodec). FFmpeg is _the_ encoder/decoder library and licensed under the gpl/lgpl 2.1. The binaries included here were compiled to comply with the lgpl. A copy of the LGPL together with instructions how the library was compiled for each platform can be found in the `legal` folder. 
* The sourcecode for this application is freely available on [github](https://github.com/kritzikratzi/Oscilloscope). 