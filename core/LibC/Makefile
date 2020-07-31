################################################################################
### Makefile for compiling C files into shared libraries that SimForth can
### load and convert functions into a Forth words.
################################################################################

################################################################################
### Check if SRCS=filename.c refering to the generated file to compile is passed
### as parameter to Makefile.
ifeq ($(SRCS),)
$(error "Missing passing C file name as makefile parameter. Do something like: make SRCS=library-name.c")
endif

################################################################################
### Check if BUILD=xxx refering to the folder holding compiled file is passed as
### parameter to Makefile.
ifeq ($(BUILD),)
$(error "Missing passing build directory name as makefile parameter. Do something like: make BUILD=/tmp")
endif

################################################################################
### Detect the operating system: Unix, OSX and Windows
ifeq ($(OS),Windows_NT)
ARCHI := Windows
else
ARCHI := $(shell uname -s)
endif

################################################################################
### Shut up Makefile when changing of firectories!
MAKEFLAGS += --no-print-directory

################################################################################
### List of compiled files
OBJS = $(SRCS:.c=.o)

################################################################################
### Search c files in given directories.
VPATH = $(BUILD):.

################################################################################
### Compiler definition. Note: If you prefer clang: make CC=clang-7
CC = gcc
CFLAGS ?= -W -Wall -Wextra

################################################################################
### Selection compilation flags and shared library file extension depending on
### the current architecture.
ifeq ($(ARCHI),Darwin)
DLL_CXXFLAGS = -fPIC
DLL_LDFLAGS = -dynamiclib -undefined suppress -flat_namespace
SO = dylib
else ifeq ($(ARCHI),Linux)
DLL_CXXFLAGS = -fPIC
DLL_LDFLAGS = -shared
SO = so
else ifeq ($(ARCHI),Windows)
DLL_CXXFLAGS = -fPIC
DLL_LDFLAGS = -shared -Wl,--out-implib,$(patsubst %.o,%.a,$@)
SO = dll
else
$(error I dunno how to compile dynamic lib with this architecture)
endif

################################################################################
### Get content of the pkg-config in case of no libs
ifneq ($(PKGCONFIG),)
PKGCONFIG_LIBS = $(shell pkg-config $(PKGCONFIG) --libs)
PKGCONFIG_CFLAGS = $(shell pkg-config $(PKGCONFIG) --cflags)
endif

################################################################################
### Define colors for colorful messages
CLR_DEFAULT = $(shell echo '\033[00m')
CLR_RED     = $(shell echo '\033[31m')
CLR_GREEN   = $(shell echo '\033[32m')
CLR_YELLOW  = $(shell echo '\033[33m')
CLR_BLUE    = $(shell echo '\033[34m')
CLR_PURPLE  = $(shell echo '\033[35m')
CLR_CYAN    = $(shell echo '\033[36m')

################################################################################
### Select the verbosity of Makefile
ifeq ($(VERBOSE),)
  ifeq ($(V),)
    Q := @
  else
    Q :=
  endif
else
Q :=
endif

################################################################################
### Display predefined colorful messages
print-simple = \
	@echo "$(CLR_PURPLE)*** $1:$(CLR_DEFAULT) $(CLR_CYAN)$2$(CLR_DEFAULT)"
print-from = \
	@echo "$(CLR_PURPLE)*** $1:$(CLR_DEFAULT) $(CLR_CYAN)$2$(CLR_DEFAULT) <= $(CLR_YELLOW)$3$(CLR_DEFAULT)"
print-to = \
	@echo "$(CLR_PURPLE)*** $1:$(CLR_DEFAULT) $(CLR_CYAN)$2$(CLR_DEFAULT) => $(CLR_YELLOW)$3$(CLR_DEFAULT) $4"

################################################################################
### Compile a dynamic library
all: $(OBJS)

%.o: %.c
	@$(call print-to,"Compiling","$<","$(abspath $(BUILD)/$@)")
	$(Q)$(CC) $(CFLAGS) $(DLL_CXXFLAGS) -c $(abspath $<) -o $(abspath $(BUILD)/$@) $(PKGCONFIG_CFLAGS)
	@$(call print-to,"Linking","$@","$(patsubst %.o,%.$(SO),$(abspath $(BUILD)/$@))")
	$(Q)$(CC) $(DLL_LDFLAGS) $(abspath $(BUILD)/$@) -o $(patsubst %.o,%.$(SO),$(abspath $(BUILD)/$@)) $(EXTLIBS) $(PKGCONFIG_LIBS)

################################################################################
### Clean compiled files
.PHONY: clean
clean:
	@$(call print-simple,"Cleaning","$(BUILD)")
	$(Q)cd $(BUILD) && rm -fr *~ *.o *.dylib *.dll *.so

################################################################################
### Display the compiler version and information.
.PHONY: which-compiler
$(OBJS): | which-compiler
which-compiler:
ifeq ($(Q),)
	@$(call print-simple,"Version","$(CC)")
	@$(CC) --version
endif
