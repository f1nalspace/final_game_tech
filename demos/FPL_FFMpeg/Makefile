DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS =-g3 -DDEBUG
else
	CFLAGS=-DNDEBUG
endif

BUILD_BASE_DIR =../bin/FPL_FFMpeg
SOURCE_FILES = fpl_ffmpeg.cpp
EXECUTABLE = FPL_FFMpeg
LIBS = -ldl

ifeq ($(DEBUG), 1)
	BUILD_DIR = $(BUILD_BASE_DIR)/Linux-x64-debug
else
	BUILD_DIR = $(BUILD_BASE_DIR)/Linux-x64-release
endif

all: clean prepare build

prepare:
	mkdir -p $(BUILD_DIR)

build:
	g++ -std=c++11 $(CFLAGS) -I../../ $(SOURCE_FILES) $(LIBS) -o $(BUILD_DIR)/$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR)
