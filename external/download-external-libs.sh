#!/bin/bash

### This script will git clone some needed libraries for the project.

### $1 is given by ../Makefile and refers to the current architecture.
if [ "$1" == "" ]; then
  echo "Expected one argument. Select the architecture: Linux, Darwin or Windows"
  exit 1
fi
ARCHI=$1
TARGET=$2

### Delete all previous directories to be sure to have and compile
### fresh code source.
rm -fr MyLogger Exception TerminalColor backward-cpp 2> /dev/null

function print-clone
{
    echo -e "\033[35m*** Downloading:\033[00m \033[36m$TARGET\033[00m <= \033[33m$1\033[00m"
}

### Clone Backward tool: A beautiful stack trace pretty printer for C++. No installation
### is needed but some libraries are needed (libdw-dev, binutils-dev, ...).
### License: MIT
print-clone backward-cpp
git clone https://github.com/Lecrapouille/backward-cpp.git --depth=1 > /dev/null && touch backward-cpp/.downloaded

### TerminalColor
print-clone TerminalColor
git clone https://github.com/Lecrapouille/TerminalColor --depth=1 > /dev/null && touch TerminalColor/.downloaded

### Exception
print-clone MyLogger
git clone https://github.com/Lecrapouille/MyLogger --depth=1 > /dev/null && touch MyLogger/.downloaded

### Exception
print-clone Exception
git clone https://github.com/Lecrapouille/Exception --depth=1 > /dev/null && touch Exception/.downloaded

touch .downloaded
