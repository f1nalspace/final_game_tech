# Project
APP_NAME = FPL_ImGui
IMGUI_PATH = ../dependencies/imgui/include/
SOURCE_FILES = \
	fpl_imgui.cpp \
	$(IMGUI_PATH)imgui/imgui.cpp \
	$(IMGUI_PATH)imgui/imgui_draw.cpp \
	$(IMGUI_PATH)imgui/imgui_demo.cpp
LIBS = -lm -ldl -lGL
INCLUDES = -I../../ -I$(IMGUI_PATH)

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
	#g++ -std=c++11 -g -rdynamic -Wall -fPIC $(CFLAGS) $(INCLUDES) $(SOURCE_FILES) $(LIBS) -o $(BUILD_DIR)/$(EXECUTABLE)
	g++ -std=c++11 -g3 -Wall $(CFLAGS) $(INCLUDES) $(SOURCE_FILES) $(LIBS) -o $(BUILD_DIR)/$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR)
