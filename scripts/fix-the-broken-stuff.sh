#!/bin/bash

pushd `dirname $0`

echo "---------------------------"
echo "-                         -"
echo "-      Experimental,      -"
echo "-        Temporary        -"
echo "-       Workarounds       -"
echo "-                         -"
echo "---------------------------"
echo "       By DJ_Level_3       "
echo ""
cd ..

cp -v -n addons/ofxLiblaserdock/src/LaserdockDeviceManager.h addons/ofxLiblaserdock/src/LaserdockDeviceManager.h.bak
cp -v -n addons/ofxNative/src/ofxNative_osx.mm addons/ofxNative/src/ofxNative_osx.mm.bak

cp -v addons/ofxLiblaserdock/src/LaserdockDeviceManager.h.bak addons/ofxLiblaserdock/src/LaserdockDeviceManager.h
cp -v addons/ofxNative/src/ofxNative_osx.mm.bak addons/ofxNative/src/ofxNative_osx.mm


sed -i -e '0,/#include <vector>/s//#include <vector>\n#include <cstdio>/' addons/ofxLiblaserdock/src/LaserdockDeviceManager.h

sed -i -e '0,/*\//s//*/' -e '$a*/' addons/ofxNative/src/ofxNative_osx.mm
