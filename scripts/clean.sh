#! /bin/bash

pushd `pwd`/../bin/
echo "Cleaning out bin folder ... "

echo "----------------------------"
git clean -ndx .
echo "----------------------------"

echo "Delete all the above files? "
read -p "Are you sure? [y/N] " -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
	echo "  ... Cleaning up"
	git clean -fdx .
else
	echo "  ... skipped"
fi
popd