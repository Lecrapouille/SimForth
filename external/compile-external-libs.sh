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

source ../.makefile/compile-external-libs.sh

### Compile GNU ncurses
print-compile ncurses
(cd ncurses
    call-configure --with-build-cc=$CC --with-build-cpp=$CXX
    call-make CFLAGS="-fPIC"
)

### Compile GNU Readline
print-compile readline
(cd readline
    call-configure
    call-make CFLAGS="-fPIC"
)

### Basic logger for my GitHub C++ projects
print-compile MyLogger
(cd MyLogger
    call-make)
