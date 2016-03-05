# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
    OF_ROOT=../../..
endif

copy_osx_files: 
	cp -r $(OF_ROOT)/addons/ofxMightyUI/bin/data/ bin/$(APPNAME).app/Contents/Resources/
	cp -r $(OF_ROOT)/addons/ofxFontAwesome/bin/data/ bin/$(APPNAME).app/Contents/Resources/
	

PROJECT_AFTER_OSX=make copy_osx_files

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk
