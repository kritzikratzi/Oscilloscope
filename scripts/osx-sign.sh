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


app="$1"
signingIdentity="$2"
entitlements="$3"

if [[ -z "$1" || -z "$2" || -z "$3" ]]
then
	echo "Usage: $0 <application binary> <signing-identity> <entitlements>"
	exit
fi

app="$1"
signingIdentity="$2"
entitlements="$3"
me=$(dirname $0)

# you might want extra_options="--options=runtime"
# to sign an older binary which needs to be notarized
extra_options="--options=runtime --entitlements $entitlements "


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
	        sudo codesign $extra_options -f -s "$signingIdentity" -vvvv "$FOO"
	    fi
	done)
	# Sign most other files
	find "$app" -type f -print | sort -r \
	| grep -v '_CodeSignature' \
	| (while read FOO; do
	    codesign -d "$FOO"
	    if [ $? != 0 ]; then
			echo "Signing: $FOO"
	        sudo codesign $extra_options -f -s "$signingIdentity" -vvvv "$FOO" 2>&1
	    fi
	done)
	# Sanity check, this doesn't sign anything, because everything is signed
	find "$app/$pluginName" -type f -print | sort -r \
	| ( while read FOO; do
	    codesign -v "$FOO"
	    if [ $? != 0 ]; then
			echo "Checking: $FOO"
	        sudo codesign $extra_options -f -s "$signingIdentity" -vvvv "$FOO" 2>&1
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
	        sudo codesign $extra_options -s "$signingIdentity" -vvvv "$FOO" 2>&1
	    fi
	done)
}

signdir "Contents/Frameworks/GLUT.framework"
#signdir ffmpeg.mac

# finally sign $app
sudo codesign $extra_options -s "$signingIdentity" -vvvv --force "$app"
# Now the verification works
codesign -vvvv --deep "$app"