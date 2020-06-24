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
TARGET = $(PROJECT)-gui
DESCRIPTION = GUI Forth interpreter
BUILD_TYPE = debug

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
OBJS = BaseWindow.o Application.o CloseLabel.o TextDocument.o \
  ForthInspector.o TextEditor.o ForthEditor.o ForthWindow.o main.o



###################################################
# Project defines
#

# Disable ugly gtkmm compilation warnings
DEFINES += -DGTK_SOURCE_H_INSIDE -DGTK_SOURCE_COMPILATION
DEFINES += -Wno-float-equal -Wno-pedantic -Wno-sign-conversion -Wno-switch-enum -Wno-conversion -Wno-old-style-cast -Wno-cast-qual

###################################################
# Set Libraries compiled in the external/ directory.
# For knowing which libraries is needed please read
# the doc/Install.md file.
THIRDPART_LIBS += $(abspath $(P)/$(BUILD)/libsimforth.so.$(shell cat $(P)/VERSION))

###################################################
# Set Libraries.
PKG_LIBS += gtkmm-3.0 gtksourceviewmm-3.0

###################################################
# Set Libraries.
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
