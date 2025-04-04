# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------

MY_PROJECT = FPL_SimpleAudio
MY_TRANSLATION_UNITS = fpl_simpleaudio.c
MY_LIBS = -lm
MY_INCLUDES = -I../../
MY_C_STANDARD = c99
MY_C_FLAGS = -Wall

# -----------------------------------------------------------------------------
#
# Do not change the following lines
#
# -----------------------------------------------------------------------------

# Auto detect OS and Target
ifeq ($(OS),Windows_NT)
	FPL_PLATFORM := Win32
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
		FPL_ARCH := x64
	else
		FPL_ARCH := x86
	endif
else
	FPL_PLATFORM := $(shell uname -s)
	FPL_ARCH := $(shell uname -m)
endif

FPL_COMPILER = gcc
FPL_EXECUTABLE_NAME = $(MY_PROJECT)
FPL_BUILD_BASE_DIR = ../build/$(MY_PROJECT)
FPL_DEBUG_NAME = Debug
FPL_RELEASE_NAME = Release
FPL_DEBUG_CFLAGS = -g3 -DDEBUG
FPL_RELEASE_CFLAGS = -o3 -DNDEBUG
FPL_DEBUG_BUILD_DIR = $(FPL_BUILD_BASE_DIR)/$(FPL_PLATFORM)-$(FPL_ARCH)-$(FPL_DEBUG_NAME)
FPL_RELEASE_BUILD_DIR = $(FPL_BUILD_BASE_DIR)/$(FPL_PLATFORM)-$(FPL_ARCH)-$(FPL_RELEASE_NAME)

.PHONY: all build clean_debug debug clean_release release clean

all: clean release

build: clean release

clean_debug:
	rm -rf $(FPL_DEBUG_BUILD_DIR)

clean_release:
	rm -rf $(FPL_RELEASE_BUILD_DIR)

clean: clean_debug clean_release

debug: clean_debug
	mkdir -p $(FPL_DEBUG_BUILD_DIR)
	$(FPL_COMPILER) -std=$(MY_C_STANDARD) $(MY_C_FLAGS) $(FPL_DEBUG_CFLAGS) $(MY_INCLUDES) $(MY_TRANSLATION_UNITS) -ldl $(MY_LIBS) -o $(FPL_DEBUG_BUILD_DIR)/$(FPL_EXECUTABLE_NAME)

release: clean_release
	mkdir -p $(FPL_RELEASE_BUILD_DIR)
	$(FPL_COMPILER) -std=$(MY_C_STANDARD) $(MY_C_FLAGS) $(FPL_RELEASE_CFLAGS) $(MY_INCLUDES) $(MY_TRANSLATION_UNITS) -ldl $(MY_LIBS) -o $(FPL_RELEASE_BUILD_DIR)/$(FPL_EXECUTABLE_NAME)

