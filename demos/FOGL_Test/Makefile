# Project
APP_NAME = FOGL_Test
SOURCE_FILES = fogl_test.c
LIBS = -ldl
INCLUDES = -I../../

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
BUILD_BASE_DIR =../bin/$(APP_NAME)
EXECUTABLE = $(APP_NAME)
BUILD_DIR = $(BUILD_BASE_DIR)/$(PLAFORM_NAME)-$(ARCH_TYPE)-$(RELEASE_TYPE)

all: clean prepare build

prepare:
	mkdir -p $(BUILD_DIR)

build:
	gcc -std=c99 $(CFLAGS) $(INCLUDES) $(SOURCE_FILES) $(LIBS) -o $(BUILD_DIR)/$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR)
