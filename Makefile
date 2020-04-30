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
DESCRIPTION = A basic Forth for SimTaDyn
BUILD_TYPE = debug
#USE_GPROF=1

###################################################
# Location of the project directory and Makefiles
#
P := .
M := $(P)/.makefile
include $(M)/Makefile.header

###################################################
# Inform Makefile where to find header files
#
INCLUDES += \
  -I$(P)/include -I$(P)/src -I$(P)/src/utils

###################################################
# Inform Makefile where to find *.cpp and *.o files
#
VPATH += \
  $(P)/src $(P)/src/utils $(P)/src/standalone

###################################################
# Make the list of compiled files
#
OBJS_UTILS = \
  Exception.o File.o ILogger.o Logger.o Path.o
OBJS_FORTH = \
  Utils.o Exceptions.o LibC.o Streams.o Dictionary.o Display.o \
  Interpreter.o Primitives.o Forth.o
OBJS = $(OBJS_UTILS) $(OBJS_FORTH) main.o

###################################################
# Project defines
#
DEFINES += \
  -DPROJECT_TEMP_DIR=\"/tmp/$(TARGET)/\" \
  -DPROJECT_DATA_PATH=\"$(PWD)/core:$(PROJECT_DATA_ROOT)/core\" \
  -DDYLIB_EXT=\".$(SO)\"

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
# Compile and launch unit tests and generate the code coverage html report.
.PHONY: unit-tests
unit-tests:
	@$(call print-simple,"Compiling unit tests")
	@$(MAKE) -C tests coverage

###################################################
# Compile and launch unit tests and generate the code coverage html report.
.PHONY: check
check: unit-tests

###################################################
# Install project. You need to be root.
.PHONY: install
install: $(TARGET)
	@$(call INSTALL_DOCUMENTATION)
	@$(call INSTALL_PROJECT_FOLDER,core)

###################################################
# Uninstall the project. You need to be root. FIXME: to be updated
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
	@(cd tests && $(MAKE) -s clean)
	@$(call print-simple,"Cleaning","$(PWD)/doc/html")
	@rm -fr $(THIRDPART)/*/ doc/html 2> /dev/null

###################################################
# Sharable informations between all Makefiles
include $(M)/Makefile.footer
