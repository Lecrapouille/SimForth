#!/bin/bash -e

###############################################################################
### This script is called by (cd .. && make download-external-libs). It will
### git clone thirdparts needed for this project but does not compile them.
### It replaces git submodules that I dislike.
###############################################################################

### $1 is given by ../Makefile and refers to the current architecture.
if [ "$1" == "" ]; then
  echo "Expected one argument. Select the architecture: Linux, Darwin or Windows"
  exit 1
fi
ARCHI="$1"
TARGET="$2"

### Delete all previous directories to be sure to have and compile fresh code source.
rm -fr MyLogger Exception TerminalColor backward-cpp ncurses readline 2> /dev/null

### Git clone a GitHub repository $1
URL="github.com"
function cloning
{
    REPO="$1"
    shift

    echo -e "\033[35m*** Cloning: \033[36m$URL/$REPO\033[00m >= \033[33m$TARGET\033[00m"
    git clone https://$URL/$REPO.git --depth=1 --recurse $* > /dev/null
}

### TerminalColor
cloning Lecrapouille/TerminalColor

### Exception
cloning Lecrapouille/MyLogger

### Exception
cloning Lecrapouille/Exception

### GNU ncurses
cloning mirror/ncurses

### GNU Readline
URL="git.savannah.gnu.org/git"
cloning readline

