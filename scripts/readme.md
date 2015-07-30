Helper Scripts
===


These scripts help manage the build process. 
You'll need a bash shell to run the scripts. 

The scripts should all figure out the correct paths themselves. There's no need to worry about the working directory. 


clean.sh
---

Cleans out the bin/ folder and removes files not in the repository. 
Changed files will not be reset. 

prepare.sh
---
Prepares the bin folder for a platform. This also fixes some problems with the build process that (i believe) I'm currently too dumb to fix directly in the make files. 

The script requires one argument, the operating system. Possible options are: 

* osx
* linux
* linux64
* win32
* win64

Depending on the parameter some dll files and other resources are copied to the bin folder. 
Use `clean.sh` to get rid of the mess again. 


dist.sh
---
Assembles a zip file ready for binary distribution. This adds the ffmpeg license files, compilation instructions, and does a whole lot of other things. It takes two parameters, one for the version, the other for the platform. 

A typical distribution workflow looks like this: 

	cd apps/myApps/Oscilloscope
	
	platform=osx
	version=1.0.3
	
	dist/clean.sh
	dist/prepare.sh $platform
	
	# now build & test (use xcode on mac, visual studio on windows, run make release on linux)
	
	dist/dist.sh $platform $version
	
	