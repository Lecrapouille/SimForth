#!/bin/bash -e
###############################################################################
### This script is called by (cd .. && make download-external-libs). It will
### git clone thirdparts needed for this project but does not compile them.
### It replaces git submodules that I dislike.
###############################################################################

source ../.makefile/download-external-libs.sh

### TerminalColor
cloning Lecrapouille/TerminalColor

### Exception
cloning Lecrapouille/MyLogger

### Exception
cloning Lecrapouille/Exception

### GNU ncurses
cloning mirror/ncurses

### GNU Readline
URL=$SAVANA_URL
cloning readline

