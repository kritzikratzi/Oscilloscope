#! /bin/bash

version=$2
platform=$1

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



mkdir dist
cp -R bin dist/Oscilloscope

cp -R docs dist/Oscilloscope
mkdir dist/Oscilloscope/docs/ffmpeg
cp -R ../../../addons/ofxAvCodec/ffmpeg_src/readme.md dist/Oscilloscope/docs/ffmpeg/notes.md
cp -R ../../../addons/ofxAvCodec/libs/avcodec/LICENSE.md dist/Oscilloscope/docs/ffmpeg/license.md
cp -R ../../../addons/ofxAvCodec/libs/avcodec/README.md dist/Oscilloscope/docs/ffmpeg/readme.md
cp readme.md dist/Oscilloscope
cd dist


dest="Oscilloscope-${version}-${platform}.zip"
zip --symlinks -r $dest Oscilloscope

echo "----------------------------"
echo "Generated ${dest}"
echo `du -h Oscilloscope | tail -n 1 | cut -f 1`
echo "`du -h $dest | tail -n 1 | cut -f 1` (compressed)"
echo "----------------------------"
popd
