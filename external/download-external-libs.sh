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

### Delete all previous directories to be sure to have and compile
### fresh code source.
rm -fr MyLogger Exception TerminalColor backward-cpp 2> /dev/null

function print-clone
{
    echo -e "\033[35m*** Cloning:\033[00m \033[36m$TARGET\033[00m <= \033[33m$1\033[00m"
}

### TerminalColor
print-clone TerminalColor
git clone https://github.com/Lecrapouille/TerminalColor --depth=1 > /dev/null

### Exception
print-clone MyLogger
git clone https://github.com/Lecrapouille/MyLogger --depth=1 > /dev/null

### Exception
print-clone Exception
git clone https://github.com/Lecrapouille/Exception --depth=1 > /dev/null
