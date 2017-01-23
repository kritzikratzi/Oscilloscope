#! /bin/bash

platform=$1
version=$2

if [[ -z "$version" || -z "$platform" ]]
then
	echo "Usage: dist.sh <platform> <version>"
	echo ""
	echo "Platform: One of the following: "
	echo "          linux, linux64, win32, win64" 
	echo "Version:  Anything you like"
	exit
fi


pushd `dirname $0`
cd ..

if [ -d "dist/Oscilloscope" ]
then
	echo "Cleaning out work folder first ..."
	rm -rf dist/Oscilloscope
fi



mkdir -p dist
cp -R bin dist/Oscilloscope

cp -R docs dist/Oscilloscope
mkdir -p dist/Oscilloscope/docs/ffmpeg
cp -R addons/ofxAvCodec/ffmpeg_src/readme.md dist/Oscilloscope/docs/ffmpeg/notes.md
cp -R addons/ofxAvCodec/libs/avcodec/LICENSE.md dist/Oscilloscope/docs/ffmpeg/license.md
cp -R addons/ofxAvCodec/libs/avcodec/README.md dist/Oscilloscope/docs/ffmpeg/readme.md
cp readme.md dist/Oscilloscope
cd dist

echo "platform = $platform"

if [ "$platform" = "win32" ]
then
	cd Oscilloscope
	dlls="assimp.dll glut32.dll libeay32.dll ssleay32.dll swscale-3.dll Zlib.dll FreeType.dll fmodex.dll fmodexL.dll"
	echo "Deleting unused DLL files: $dlls "
	echo "Make sure they are specified as 'delay loaded DLLs' in the linker settings"
	rm $dlls
	cd ..
elif [ "$platform" = "osx" ]
then
	echo "Moving data folder into resources"
	cd Oscilloscope
	mv data/* Oscilloscope.app/Contents/Resources
	rm -rf data
	
	echo "Stripping all dylibs to 64bit only"
	for file in $(find Oscilloscope.app -type f -name "*.dylib")
	do
		echo "    > processing " $(basename $file)
		mv "$file" /tmp/oscilloscope-fatlib.dylib
		lipo /tmp/oscilloscope-fatlib.dylib -thin x86_64 -output "$file"
		rm /tmp/oscilloscope-fatlib.dylib
	done
	
	cd ..
fi

echo "----------------------------"
echo "Generated ${dest}"
echo `du -h Oscilloscope | tail -n 1 | cut -f 1`
echo "----------------------------"
popd
