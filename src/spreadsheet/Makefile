PROJECT = SimForth
TARGET = SpreadSheet
DESCRIPTION = Spreadsheet using Forth instead of Visual Basic
BUILD_TYPE = debug

P=../..
M=$(P)/.makefile
include $(M)/Makefile.header

OBJS += main.o
PKG_LIBS += simforth gtkmm-3.0
VPATH += .
INCLUDES += -I.

all: $(TARGET)

include $(M)/Makefile.footer
