#!/bin/sh 
realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH"
}

if [ -z "$1" ]
then
	echo "Usage: $0 <application binary> <signing-identity>"
	echo "No binary provided"
	exit
fi

if [ -z "$2" ]
then
	echo "Usage: $0 <application binary> <signing-identity>"
	echo "No signing identity provided. "
	echo "Searching for available options..."
	security find-identity -v -p codesigning
	exit
fi

app="$1"
signingIdentity="$2"

echo "Clearing strange file attributes"
xattr -cr "$app"

signdir() {
	pluginName=$1
	echo "Signing $app/$pluginName"
	# sign bundled jre
	find "$app/$pluginName" -type f -print | sort -r \
	| ( while read FOO; do
	    codesign -d "$FOO"
	    if [ $? != 0 ]; then
	        sudo codesign --options=runtime -f -s "$signingIdentity" -vvvv "$FOO"
	    fi
	done)
	# Sign most other files
	find "$app" -type f -print | sort -r \
	| grep -v '_CodeSignature' \
	| (while read FOO; do
	    codesign -d "$FOO"
	    if [ $? != 0 ]; then
			echo "Signing: $FOO"
	        sudo codesign --options=runtime -f -s "$signingIdentity" -vvvv "$FOO" 2>&1
	    fi
	done)
	# Sanity check, this doesn't sign anything, because everything is signed
	find "$app/$pluginName" -type f -print | sort -r \
	| ( while read FOO; do
	    codesign -v "$FOO"
	    if [ $? != 0 ]; then
			echo "Checking: $FOO"
	        sudo codesign --options=runtime -f -s "$signingIdentity" -vvvv "$FOO" 2>&1
	    fi
	done)
	# Test sig
	codesign -dvvv "$app/$pluginName"
	# Next sign Java directory (must not use --force, as plist is modified)
	# This creates $app/Contents/Plugins/Java/Contents/_CodeSignature/{CodeDirectory,CodeRequirements,CodeResources,CodeSignature}
	sudo codesign -f -s "$signingIdentity" -vvvv "$app/$pluginName" 2>&1
	sudo codesign -f -s "$signingIdentity" "$app/$pluginName/Contents/_CodeSignature/CodeResources"
	# sign the new files
	find "$app/$pluginName" -type f -print | sort -r \
	| ( while read FOO; do
	    codesign -v "$FOO"
	    if [ $? != 0 ]; then
	        sudo codesign --options=runtime -s "$signingIdentity" -vvvv "$FOO" 2>&1
	    fi
	done)
}

signdir "Contents/Frameworks/GLUT.framework"
#signdir ffmpeg.mac

# finally sign $app
sudo codesign --options=runtime -s "$signingIdentity" -vvvv --force "$app"
# Now the verification works
codesign -vvvv --deep "$app"