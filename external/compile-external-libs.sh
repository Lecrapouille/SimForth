#!/bin/bash -e

###############################################################################
### This script is called by (cd .. && make compile-external-libs). It will
### compile thirdparts cloned previously with make download-external-libs.
###
### To avoid pollution, these libraries are not installed in your operating
### system (no sudo make install is called). As consequence, you have to tell
### your project ../Makefile where to find their files. Here generic example:
###     INCLUDES += -I$(THIRDPART)/thirdpart1/path1
###        for searching heeder files.
###     VPATH += $(THIRDPART)/thirdpart1/path1
###        for searching c/c++ files.
###     THIRDPART_LIBS += $(abspath $(THIRDPART)/libXXX.a))
###        for linking your project against the lib.
###     THIRDPART_OBJS += foo.o
###        for inking your project against this file iff THIRDPART_LIBS is not
###        used (the path is not important thanks to VPATH).
###
### The last important point to avoid polution, better to compile thirdparts as
### static library rather than shared lib to avoid telling your system where to
### find them when you'll start your application.
###############################################################################

### $1 is given by ../Makefile and refers to the current architecture.
if [ "$1" == "" ]; then
  echo "Expected one argument. Select the architecture: Linux, Darwin or Windows"
  exit 1
fi

ARCHI="$1"
TARGET="$2"
CC="$3"
CXX="$4"

function print-compile
{
    echo -e "\033[35m*** Compiling:\033[00m \033[36m$TARGET\033[00m <= \033[33m$1\033[00m"
}

### Number of CPU cores
NPROC=
if [[ "$ARCHI" == "Darwin" ]]; then
    NPROC=`sysctl -n hw.logicalcpu`
else
    NPROC=`nproc`
fi

### Compile GNU ncurses
print-compile ncurses
if [ -e readline ];
then
    (
        cd ncurses
        ./configure --with-build-cc=$CC --with-build-cpp=$CXX
        VERBOSE=1 make CFLAGS="-fPIC" -j$NPROC
    )
else
    echo "Failed compiling external/readline: directory does not exist"
fi

### Compile GNU Readline
print-compile readline
if [ -e readline ];
then
    (
        cd readline
        export CXX=$CXX
        export CC=$CC
        ./configure
        VERBOSE=1 make CFLAGS="-fPIC" -j$NPROC
    )
else
    echo "Failed compiling external/readline: directory does not exist"
fi

### Basic logger for my GitHub C++ projects
print-compile MyLogger
if [ -e MyLogger ];
then
    (cd MyLogger
     VERBOSE=1 make CXX=$CXX CC=$CC -j$NPROC
    )
else
    echo "Failed compiling external/SOIL: directory does not exist"
fi
