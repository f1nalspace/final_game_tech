# Project
APP_NAME = FPL_MiniAL
SOURCE_FILES = fpl_miniaudio.c
LIBS = -ldl -lm
INCLUDES = -I../../ -I../additions/ -I../dependencies/

# Auto detect release type/platform/architecture
DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS =-g3 -DDEBUG
	RELEASE_TYPE = debug
else
	CFLAGS=-DNDEBUG
	RELEASE_TYPE = release
endif
ARCH_TYPE = x64
PLAFORM_NAME = Linux

# Do not modify starting
BUILD_BASE_DIR =../build/$(APP_NAME)
EXECUTABLE = $(APP_NAME)
BUILD_DIR = $(BUILD_BASE_DIR)/$(PLAFORM_NAME)-$(ARCH_TYPE)-$(RELEASE_TYPE)

all: clean prepare build

prepare:
	mkdir -p $(BUILD_DIR)

build:
	gcc -std=c99 $(CFLAGS) $(INCLUDES) $(SOURCE_FILES) $(LIBS) -o $(BUILD_DIR)/$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR)
