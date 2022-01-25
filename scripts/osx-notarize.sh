#!/bin/sh
# notarizes and zip a signed app
# run this in the folder where your app sits! 
# otherwise you end up with things in weird places
appFolder="$1"
bundleId="$2"
teamId="$3"
ascProvider="$4"
username="$5"
password="$6"
zipDest="$7"

if [[ -z "$appFolder" || -z "$bundleId" || -z "$username" || -z "$password" || -z "$zipDest"  || -z "$teamId" ]]
then
	echo "Usage: $0 <application binary> <bundle-id> <team-id> <asc-provider> <username> <password> <zip-destination>"
	exit
fi

appFolderName=$(basename "$appFolder")
appDest="$distDir/$appFolderName"
appName=${appFolderName%.app}
me="$(dirname $0)"

echo "Zipping it up..."
if [ -f "$zipDest" ]
then
	rm "$zipDest"
fi
ditto -c -k --keepParent "$appName.app" "$zipDest"

echo "Uploading for notarization..."
echo xcrun altool -asc-provider $ascProvider --team-id $teamId --notarize-app --primary-bundle-id "$bundleId" --username "$username" --password "$password" --file "$zipDest" 
uuid="$(xcrun altool -asc-provider $ascProvider --team-id $teamId --notarize-app --primary-bundle-id "$bundleId" --username "$username" --password "$password" --file "$zipDest" 2>&1 | grep RequestUUID | cut -d " " -f 3)"

if [ -z "$uuid" ]
then
	echo "Got no uuid ($uuid) as output of altool, exiting"
	exit 1
else
	echo "Got uuid $uuid from altool"
fi

while true
do
	output="$(xcrun altool --team-id $teamId --notarization-info "$uuid" -u "$username" -p "$password" 2>&1)"
	res=$(echo "$output" | grep "Status Message" | xargs | cut -d " " -f 3-99)
	if [ -z "$res" ]
	then
		echo " > waiting for notarization..."
	else
		if [ "$res" == "Package Approved" ]
		then
			echo " > Package check ok!"
			break
		else
			url=$(echo "$output" | grep "LogFileURL" | xargs | cut -d " " -f 2-99)
			curl $url
			echo " > Package not ok, got \"$output\""
			exit 1
		fi
	fi
done

echo "Deleting zip"
rm "$zipDest"
echo "Running stapler on the app"
xcrun stapler staple "$appName.app"
echo "Recreating final zip"
ditto -c -k --keepParent "$appName.app" "$zipDest"

	


