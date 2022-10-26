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
DESCRIPTION = Forth Library for C++ project
BUILD_TYPE = release

###################################################
# Location of the project directory and Makefiles
#
P := .
M := $(P)/.makefile
include $(M)/Makefile.header

###################################################
# Inform Makefile where to find header files
#
INCLUDES += -I$(P)/include -I$(P)/src -I$(P)/external/MyLogger/include	\
-I$(P)/external/MyLogger/src -I$(P)/external

###################################################
# Inform Makefile where to find *.cpp files
#
VPATH += $(P)/src $(P)/external/Exception $(P)/external/MyLogger/src	\
  $(P)/src/standalone

###################################################
# Make the list of compiled files used both by the
# library and application
#
COMMON_OBJS = Exception.o File.o ILogger.o Logger.o Path.o Options.o Utils.o Exceptions.o LibC.o	\
  Streams.o Dictionary.o Display.o Interpreter.o Primitives.o SimForth.o

###################################################
# Make the list of compiled files for the library
#
LIB_OBJS += $(COMMON_OBJS)

###################################################
# Make the list of compiled files for the application
#
OBJS += $(COMMON_OBJS) standalone.o

###################################################
# GNU Readline
THIRDPART_LIBS += $(abspath $(THIRDPART)/readline/libreadline.a)
THIRDPART_LIBS += $(abspath $(THIRDPART)/ncurses/lib/libncurses.a)

###################################################
# Project defines
#
DEFINES += -DPROJECT_DATA_PATH=\"$(PWD)/core:$(PROJECT_DATA_ROOT)/core\"

###################################################
# Set Libraries:
# -lreadline: for interactive prompt
# -ldl: for loading symbols in shared libraries
#
NOT_PKG_LIBS += -ldl

###################################################
# Compile the project
all: $(TARGET) $(STATIC_LIB_TARGET) $(SHARED_LIB_TARGET) $(PKG_FILE)

###################################################
# Compile, launch unit tests and generate the code coverage html report.
.PHONY: check
check:
	@$(call print-simple,"Compiling unit tests")
	@$(MAKE) -C tests coverage

###################################################
# Install project. You need to be root.
.PHONY: install
install: $(STATIC_LIB_TARGET) $(SHARED_LIB_TARGET) pkg-config
	@$(call INSTALL_DOCUMENTATION)
	@$(call INSTALL_PROJECT_LIBRARIES)
	@$(call INSTALL_PROJECT_HEADERS)
	@$(call INSTALL_PROJECT_FOLDER,core)
	@$(call INSTALL_THIRDPART_FOLDER,MyLogger/include,,-name "*.[tih]pp")
	@$(call INSTALL_THIRDPART_FOLDER,MyLogger/src,,-name "*.[tih]pp")
	@$(call INSTALL_THIRDPART_FILES,TerminalColor,TerminalColor,-name "*.hpp")
	@$(call INSTALL_THIRDPART_FILES,Exception,Exception,-name "*.hpp")

###################################################
# TODO
# Uninstall the project. You need to be root.
#.PHONY: uninstall
#uninstall:
#	@$(call print-simple,"Uninstalling",$(PREFIX)/$(TARGET))
#	@rm $(PROJECT_EXE)
#	@rm -r $(PROJECT_DATA_ROOT)

###################################################
# Clean the whole project.
.PHONY: veryclean
veryclean: clean
	@rm -fr cov-int $(PROJECT).tgz *.log foo 2> /dev/null
	@$(call print-simple,"Cleaning","$(PWD)/$(THIRDPART)")
	@rm -fr $(THIRDPART)/*/ $(THIRDPART)/.downloaded
	@(cd tests && $(MAKE) -s clean)
	@$(call print-simple,"Cleaning","generated documentation")
	@rm -fr doc/coverage doc/gprof doc/html tests/doc 2> /dev/null

###################################################
# Sharable informations between all Makefiles
include $(M)/Makefile.footer
