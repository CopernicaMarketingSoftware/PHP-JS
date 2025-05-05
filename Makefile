#
#	Makefile template
#
#	This is an example Makefile that can be used by anyone who is building
#	his or her own PHP extensions using the PHP-CPP library.
#
#	In the top part of this file we have included variables that can be
#	altered to fit your configuration, near the bottom the instructions and
#	dependencies for the compiler are defined. The deeper you get into this
#	file, the less likely it is that you will have to change anything in it.
#

#
#	Name of your extension
#
#	This is the name of your extension. Based on this extension name, the
#	name of the library file (name.so) and the name of the config file (name.ini)
#	are automatically generated
#

NAME				=	php-js


#
#	Php.ini directories
#
#	In the past, PHP used a single php.ini configuration file. Today, most
#	PHP installations use a conf.d directory that holds a set of config files,
#	one for each extension. Use this variable to specify this directory.
#
#

INI_DIR				=	$(shell php-config --ini-path)/../mods-available

#
#	The extension dirs
#
#	This is normally a directory like /usr/lib/php/20121221 (based on the
#	PHP version that you use. We make use of the command line 'php-config'
#	instruction to find out what the extension directory is, you can override
#	this with a different fixed directory
#

EXTENSION_DIR		=	$(shell php-config --extension-dir)


#
#	The name of the extension and the name of the .ini file
#
#	These two variables are based on the name of the extension. We simply add
#	a certain extension to them (.so or .ini)
#

EXTENSION 			=	${NAME}.so
INI 				=	${NAME}.ini


#
#	Compiler
#
#	By default, the GNU C++ compiler is used. If you want to use a different
#	compiler, you can change that here. You can change this for both the
#	compiler (the program that turns the c++ files into object files) and for
#	the linker (the program that links all object files into the single .so
#	library file. By default, g++ (the GNU C++ compiler) is used for both.
#

COMPILER			=	c++
LINKER				=	c++


#
#	Compiler and linker flags
#
#	- V8_COMPRESS_POINTERS is needed because the V8 library is compiled with a special optimization for pointers
#	- V8_ENABLE_SANDBOX is needed because the V8 library is compiled with sandbox support
#

COMPILER_FLAGS		=	-Wall -c -O2 -MD -std=c++20 -fpic -DVERSION="`./version.sh`" -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX -I. -g
LINKER_FLAGS		=	-shared
LINKER_DEPENDENCIES	=	-Wl,--no-as-needed -lphpcpp -lv8_libplatform -lv8


#
#	Command to remove files, copy files and create directories.
#
#	I've never encountered a *nix environment in which these commands do not work.
#	So you can probably leave this as it is
#

RM					=	rm -f
CP					=	cp -f
MKDIR					=	mkdir -p
XXD					=	xxd -i


#
#	All source files are simply all *.cpp files found in the current directory
#
#	A builtin Makefile macro is used to scan the current directory and find
#	all source files. The object files are all compiled versions of the source
#	file, with the .cpp extension being replaced by .o.
#

SOURCES				=	$(wildcard *.cpp)
OBJECTS				=	$(SOURCES:%.cpp=%.o)
DEPENDENCIES		=	$(SOURCES:%.cpp=%.d)


#
#	From here the build instructions start
#

all:					${OBJECTS} ${EXTENSION}

#
#   Use dependency tracking
#
-include ${DEPENDENCIES}

# natives_blob.h: natives_blob.bin
# 	${CP} natives_blob.bin /tmp/natives_blob.bin
# 	${XXD} /tmp/natives_blob.bin > natives_blob.h
# 	${RM} /tmp/natives_blob.bin
# 
# snapshot_blob.h: snapshot_blob.bin
# 	${CP} snapshot_blob.bin /tmp/snapshot_blob.bin
# 	${XXD} /tmp/snapshot_blob.bin > snapshot_blob.h
# 	${RM} /tmp/snapshot_blob.bin

${EXTENSION}:			${OBJECTS}
						${LINKER} ${LINKER_FLAGS} -o $@ ${OBJECTS} ${LINKER_DEPENDENCIES}

${OBJECTS}:
						${COMPILER} ${COMPILER_FLAGS} -o $@ ${@:%.o=%.cpp}

install:
						${CP} ${EXTENSION} ${EXTENSION_DIR}
						${CP} ${INI} ${INI_DIR}

clean:
						${RM} ${EXTENSION} ${OBJECTS} ${DEPENDENCIES}

