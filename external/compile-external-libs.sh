#!/bin/bash -e

### This script will git clone some libraries that SimForth needs and
### compile them. To avoid pollution, they are not installed into your
### environement. Therefore SimForth Makefiles have to know where to
### find their files (includes and static/shared libraries).

### $1 is given by ../Makefile and refers to the current architecture.
if [ "$1" == "" ]; then
  echo "Expected one argument. Select the architecture: Linux, Darwin or Windows"
  exit 1
fi

ARCHI="$1"
TARGET="$2"

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
        ./configure
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
        ./configure
        VERBOSE=1 make CFLAGS="-fPIC" -j$NPROC
    )
else
    echo "Failed compiling external/readline: directory does not exist"
fi
