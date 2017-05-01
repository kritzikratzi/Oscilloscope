Useful infos for compiling FFmpeg/libavcodec
===


Links
---

* The [compilation guide](https://trac.ffmpeg.org/wiki/CompilationGuide) official contains instructions for virtually all platforms. 
* Shared libraries for Windows: [http://ffmpeg.zeranoe.com/builds/](http://ffmpeg.zeranoe.com/builds/)
* Git repo with full source: git://source.ffmpeg.org/ffmpeg.git or from [https://github.com/FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg)
* Included binaries were built from commit [https://github.com/FFmpeg/FFmpeg/commit/d5fcca83b915df9536d595a1a44c24294b606836](https://github.com/FFmpeg/FFmpeg/commit/d5fcca83b915df9536d595a1a44c24294b606836)
* Lots of information about when which codec is built and which configure flags are required: https://www.ffmpeg.org/ffmpeg-codecs.html

Command Line
---

* ./configure --help will list a lot of options and their current values (eg. --enable-libgsm          enable GSM de/encoding via libgsm [no])
* `ffmpeg -codecs` to list of installed codecs
* `./configure --list-decoders` to list available decoders
* `./configure --list-encoders` to list available encoders


Compiling on Mac OS
---
The to create a universal binary run the following commands in the ffmpeg source directory: 

	# 1. Build 64 bit
	make clean
	./configure  --prefix=`pwd`/dist/ --enable-pic --enable-shared  --shlibdir="@executable_path" --shlibdir="@executable_path" --disable-indevs
	make && make install
	mv @executable_path libs_64
	
	# 2. Build 32 bit
	make clean
	./configure  --prefix=`pwd`/dist/ --enable-pic --enable-shared  --shlibdir="@executable_path" --shlibdir="@executable_path" --disable-indevs --extra-cflags="-m32" --extra-cxxflags="-m32" --extra-ldflags="-m32" --arch=i386
	make && make install
	mv @executable_path libs_32

	# 3. Combine dylibs into fat libs and copy over symlinks
	mkdir libs
	for file in libs_64/*.dylib
	do
		f=$(basename $file)
		if [ -h $file ];then;cp -av $file libs
		else;lipo libs_32/$f $file -output libs/$f -create
		fi
	done
	
	# 4. Clean up a bit to be ready for the next build ... 
	rm -rf libs_32 libs_64
	mv libs libs-$(git rev-parse HEAD)

Done! Now copy the include dir to ofxAvCodec/libs/avcodec/include and the libs dir to ofxAvCodec/libs/avcodec/lib/osx

Btw: Directly after running configure you get a neat list of all enabled components like codecs, muxer, and other things I've never heard of. 


|Flag|Description|
|----|-----------|
|``--prefix=`pwd`/dist/``|Sets the output path to the dist folder|
|`--enable-pic`|not sure|
|`--enable-shared`|compile as shared libraries (disables static libs)|
|`--shlibdir="@executable_path"`|tells each dylib to look for other dylibs in the same folder (i think)|
|`--disable-indevs`|Disables input devices like qtkit. I added this flag to get rid of the shared lib dependency to jack audio|
|`--extra-cflags="-m32" --extra-cxxflags="-m32" --extra-ldflags="-m32" --arch=i386`|Create 32 bit binaries|


	
* Copy only the dylibs to libs/avcodec/lib/osx (i copied the symlinks too, but not they're needed)
* Copy the header directory to libs/avcodec/include
* Make sure to keep the file libs/avcodec/include/libavutil/inttypes.h
  It's required for VS2012. This file is licensed as new bsd license. Included from [https://code.google.com/p/msinttypes/](https://code.google.com/p/msinttypes/). In the same folder in common.h replace `#include <inttypes.h>` with `#include "inttypes.h"`. 




Compiling for Windows using Mac OS
---
Compiled using the build scripts from [https://github.com/rdp/ffmpeg-windows-build-helpers](https://github.com/rdp/ffmpeg-windows-build-helpers). A great little script that sets up mingw and generates shared libraries. Create a directory `ffmpeg_src/win/` and run 
	
	wget https://raw.github.com/rdp/ffmpeg-windows-build-helpers/master/cross_compile_ffmpeg.sh -O cross_compile_ffmpeg.sh
	./cross_compile_ffmpeg.sh --build-ffmpeg-shared=y --build-ffmpeg-static=n


Before doing anything, edit the file and look for the `build_ffmpeg()` function. A few lines into the function you can find the configure options used. I modified them as follows: 

	  #original config options: 
	  #config_options="--arch=$arch --target-os=mingw32 --cross-prefix=$cross_prefix --pkg-config=pkg-config --enable-gpl --enable-libsoxr --enable-fontconfig --enable-libass --enable-libutvideo --enable-libbluray --enable-iconv --enable-libtwolame --extra-cflags=-DLIBTWOLAME_STATIC --enable-libzvbi --enable-libcaca --enable-libmodplug --extra-libs=-lstdc++ --extra-libs=-lpng --enable-libvidstab --enable-libx265 --enable-decklink --extra-libs=-loleaut32 --enable-libx264 --enable-libxvid --enable-libmp3lame --enable-version3 --enable-zlib --enable-librtmp --enable-libvorbis --enable-libtheora --enable-libspeex --enable-libopenjpeg --enable-gnutls --enable-libgsm --enable-libfreetype --enable-libopus --disable-w32threads --enable-frei0r --enable-filter=frei0r --enable-libvo-aacenc --enable-bzlib --enable-libxavs --enable-libopencore-amrnb --enable-libopencore-amrwb --enable-libvo-amrwbenc --enable-libschroedinger --enable-libvpx --enable-libilbc --enable-libwavpack --enable-libwebp --enable-libgme --enable-dxva2 --enable-libdcadec --enable-avisynth $extra_configure_opts" 
	  # new config options: 
	  config_options="--arch=$arch --target-os=mingw32 --cross-prefix=$cross_prefix --pkg-config=pkg-config --extra-libs=-lstdc++ --disable-w32threads $extra_configure_opts"
	  

Now you're ready to run the script. To the question: 
`Would you like to include non-free (non GPL compatible) libraries`, answer no (N). 

The script takes a while (1-3 hours). The resulting dll files are in <br>
`sandbox/x86_64/ffmpeg_git_shared.installed/bin/` <br>
`ffmpeg_git_shared.installed/bin/`

Run ffmpeg.exe to verify that the correct configure options where in fact used. 


<i>Sometimes the script fails because sourceforge is having serious downtime issues. I found that it's usually easy to give the script a hand by downloading the correct version of the dependency yourself, then place it in the win32 and x86_64 folders. (or just do a manual git clone if you find a mirror of the code).</i>

Compiling for Linux using Ubuntu
---

This is directly taken from the [https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu](FFmpeg guide for Ubuntu). The following steps assume a 64bit Ubuntu installation. 


	sudo apt-get update
	
	sudo apt-get -y --force-yes install git autoconf automake build-essential libass-dev \
		libfreetype6-dev libsdl1.2-dev libtheora-dev libtool libva-dev libvdpau-dev\
		libvorbis-dev libxcb1-dev libxcb-shm0-dev libxcb-xfixes0-dev pkg-config \
		texi2html zlib1g-dev gcc-multilib libc6-i386
	
	git clone git://source.ffmpeg.org/ffmpeg.git
	cd ffmpeg
	# check out specific revision if you want
	./configure  --prefix=`pwd`/dist/x86_64/ --enable-pic --enable-shared
	make && make install
	
	# now cross compile for 32 bit
	make clean 
	./configure  --prefix=`pwd`/dist/i386/ --enable-pic --enable-shared --extra-cflags="-m32" --extra-cxxflags="-m32" --extra-ldflags="-m32" --arch=i386
	make && make install
	
	
Good job! If all goes well you have two directories: `dist/x86_64` and `dist/i386`


Special Licensing Considerations
---
You can enable aac with the configuration option `--enable-nonfree`. FFmpeg has the following to say about this: 

	The Fraunhofer AAC library, FAAC and aacplus are under licenses which
	are incompatible with the GPLv2 and v3. We do not know for certain if their
	licenses are compatible with the LGPL.
	If you wish to enable these libraries, pass `--enable-nonfree` to configure.
	But note that if you enable any of these libraries the resulting binary will
	be under a complex license mix that is more restrictive than the LGPL and that
	may result in additional obligations. It is possible that these
	restrictions cause the resulting binary to be unredistributeable.

To enable certain video codecs (like x264) the `--enable-gpl` flag can be added. This means your application will have to be released under the GPL as well. 