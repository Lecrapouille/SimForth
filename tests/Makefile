#=====================================================================
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
TARGET = $(PROJECT)-UnitTest
DESCRIPTION = Unit tests for $(PROJECT)
BUILD_TYPE = debug
USE_COVERAGE = 1
DEFINES += -Wno-unused-function

###################################################
# Location of the project directory and Makefiles
#
P := ..
M := $(P)/.makefile
include $(M)/Makefile.header

###################################################
# List of files to compile.
#
OBJS  = Exception.o File.o ILogger.o Logger.o Path.o LibC.o \
  Exceptions.o Utils.o Primitives.o Dictionary.o Display.o Interpreter.o Streams.o Forth.o \
  tests-utils.o tests-stack.o tests-dictionary.o tests-streams.o tests-interpreter.o \
  tests-core.o tests-clib.o main.o

###################################################
# Project defines
#
DEFINES += -Wno-undef -Wno-keyword-macro -Wno-float-equal

###################################################
# Compilation options.
#
PKG_LIBS += gtest gmock

###################################################
# Code coverage. Comment these lines if coveraging
# is not desired.
#
LINKER_FLAGS += -lreadline -ldl

###################################################
# Inform Makefile where to find header files
#
INCLUDES += \
  -I$(P)/include -I$(P)/src -I$(P)/external/MyLogger/include \
  -I$(P)/external/MyLogger/src -I$(P)/external

###################################################
# Inform Makefile where to find *.cpp and *.o files
#
VPATH += \
  $(P)/src $(P)/external/Exception $(P)/external/MyLogger/src $(P)/src/standalone

###################################################
# Compile SimTaDyn unit tests
all: $(TARGET)

###################################################
# Run unit tests.
.PHONY: unit-tests
.PHONY: check
unit-tests check: $(TARGET)
	@$(call print-to,"Running","$(TARGET)","$(RAPPORT)","")
	$(SANITIZER) ./$(BUILD)/$(TARGET) $(TU_OPTIONS) || (cat $(TARGET).log; return 1)

###################################################
# Sharable informations between all Makefiles
include $(M)/Makefile.footer
