##=====================================================================
## SimForth: A Forth for SimTaDyn.
## Copyright 2018-2019 Quentin Quadrat <lecrapouille@gmail.com>
##
## This file is part of SimForth.
##
## SimForth is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## SimForth is distributedin the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with SimForth.  If not, see <http://www.gnu.org/licenses/>.
##=====================================================================

###################################################
# Project definition
#
PROJECT = SimForth
TARGET = $(PROJECT)
DESCRIPTION = A basic standalone Forth interpreter
# FIXME cannot use debug because of shared lib + backward_cpp library makes segfault
BUILD_TYPE = release

###################################################
# Location of the project directory and Makefiles
#
P := ../..
M := $(P)/.makefile
include $(M)/Makefile.header

###################################################
# Inform Makefile where to find header files
#
INCLUDES += \
  -I$(P)/include -I$(P)/src -I$(P)/external/MyLogger/include \
  -I$(P)/external/MyLogger/src -I$(P)/external

###################################################
# Inform Makefile where to find *.cpp and *.o files
#
VPATH += $(P)/src

###################################################
# Make the list of compiled files
#
OBJS = main.o

###################################################
# Project defines
#
DEFINES +=

###################################################
# Set Libraries compiled in the external/ directory.
# For knowing which libraries is needed please read
# the doc/Install.md file.
#FIXME: the static lib name shall be extracted from root makefile
#
THIRDPART_LIBS += $(abspath $(P)/$(BUILD)/libsimforth.$(SO).$(shell cat $(P)/VERSION))

###################################################
# Set Libraries. For knowing which libraries
# is needed please read the external/README.md file.
# lreadline: for interactive prompt
# ldl: for loading symbols in shared libraries
#
LINKER_FLAGS += -lreadline -ldl

###################################################
# Compile the project
all: $(TARGET)

###################################################
# Install project. You need to be root.
.PHONY: install
install:
	@$(call INSTALL_BINARY)

###################################################
# Sharable informations between all Makefiles
include $(M)/Makefile.footer
