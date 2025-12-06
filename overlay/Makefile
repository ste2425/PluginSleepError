TARGET = overlay

BUILD_PRX = 1

OBJS = main.o exports.o blit.o libs/core.o fonts.o stubkk.o

INCDIR   = includes
CFLAGS = -Os -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

LIBS = -lpspkubridge -lpspctrl

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

all:
	mv overlay.prx "../release/"