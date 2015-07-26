#! /bin/bash
# copies required libs/files to the bin/data folder. 
# it is suggested that you clean up first! 

pushd `dirname $0`
echo "----------------------------"
cd ..
platform=$1

if [[ "$platform" == "osx" ]]; then
	echo XCode-build manages it all. Done!
	echo "----------------------------"
elif [[ "$platform" == "linux" ]]; then
	echo "Linux (32bit)"
	echo "----------------------------"
	mkdir bin/libs
	echo Copying avcodec libs ... 
	cp -R ../../../addons/ofxAvCodec/libs/avcodec/lib/linux/* bin/libs/
	echo Copying mightyUI data ...
	cp -R ../../../addons/ofxMightyUI/bin/data/* bin/data/
	echo Copying fontawsome data ...
	cp -R ../../../addons/ofxFontAwesome/bin/data/* bin/data/
elif [[ "$platform" == "linux64" ]]; then
	echo "Linux (64 bit)"
	echo "----------------------------"
	echo Copying avcodec libs ... 
	mkdir bin/libs
	cp -R ../../../addons/ofxAvCodec/libs/avcodec/lib/linux64/* bin/libs/
	echo Copying mightyUI data ...
	cp -R ../../../addons/ofxMightyUI/bin/data/* bin/data/
	echo Copying fontawsome data ...
	cp -R ../../../addons/ofxFontAwesome/bin/data/* bin/data/
elif [[ "$platform" == "win32" ]]; then
	echo "Windows (32bit)"
	echo "----------------------------"
	echo Copying avcodec libs ... 
	cp -R ../../../addons/ofxAvCodec/libs/avcodec/lib/win32/* bin/
	echo Copying mightyUI data ...
	cp -R ../../../addons/ofxMightyUI/bin/data/* bin/data/
	echo Copying fontawsome data ...
	cp -R ../../../addons/ofxFontAwesome/bin/data/* bin/data/
elif [[ "$platform" == "win64" ]]; then
	echo "Windows (64bit)"
	echo "----------------------------"
	echo Copying avcodec libs ... 
	cp -R ../../../addons/ofxAvCodec/libs/avcodec/lib/win64/* bin/
	echo Copying mightyUI data ...
	cp -R ../../../addons/ofxMightyUI/bin/data/* bin/data/
	echo Copying fontawsome data ...
	cp -R ../../../addons/ofxFontAwesome/bin/data/* bin/data/
else
	echo Unsupported platform: $platform
fi

popd