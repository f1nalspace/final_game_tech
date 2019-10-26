cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------
project(FPL_Audio)

set(MY_C_STANDARD c99)

set(MY_EXTERNAL_LIBS
	m
	)

set(MY_INCLUDE_DIRS
	"../../"
	"../additions/"
	"../dependencies/"
	)

set(MY_HEADER_FILES
	"../../final_platform_layer.h"
	"../additions/final_audiosystem.h"
	"../additions/final_waveloader.h"
	)

set(MY_TRANSLATION_UNITS
	"fpl_audio.c"
	)

set(MY_DEFINES
	-DFPL_IMPLEMENTATION
	-DFPL_LOGGING
	-DFINAL_AUDIOSYSTEM_IMPLEMENTATION
	-DFINAL_GRAPHICS_IMPLEMENTATION
	-DFINAL_WAVELOADER_IMPLEMENTATION
	-DFINAL_DEBUG_IMPLEMENTATION
	)

# -----------------------------------------------------------------------------
#
# Do not change the following lines
#
# -----------------------------------------------------------------------------

set(FPL_ROOT_PATH_RELATIVE ../)
get_filename_component(FPL_ROOT_PATH ${FPL_ROOT_PATH_RELATIVE} ABSOLUTE)
set(FPL_EXECUTABLE_NAME ${PROJECT_NAME})
set(FPL_EXECUTABLE_PATH ${FPL_ROOT_PATH}/build/${PROJECT_NAME}/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})

message(STATUS "\n")
message(STATUS "FPL CMake Infos: ${PROJECT_NAME}")
message(STATUS "---------------------------------------------------------------")
message(STATUS "C-Standard: ${MY_C_STANDARD}")
message(STATUS "External libraries: ${MY_EXTERNAL_LIBS}")
message(STATUS "Include directories: ${MY_INCLUDE_DIRS}")
message(STATUS "Header files: ${MY_HEADER_FILES}")
message(STATUS "Translation units: ${MY_TRANSLATION_UNITS}")
message(STATUS "Defines: ${MY_DEFINES}")
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Root dir: ${FPL_ROOT_PATH}")
message(STATUS "Executable path: ${FPL_EXECUTABLE_PATH}")
message(STATUS "Executable name: ${FPL_EXECUTABLE_NAME}")
message(STATUS "---------------------------------------------------------------\n")

set(CMAKE_C_FLAGS "-std=${MY_C_STANDARD}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${FPL_EXECUTABLE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${FPL_EXECUTABLE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${FPL_EXECUTABLE_PATH})

add_definitions(${MY_DEFINES})

include_directories(../../ ${MY_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${MY_TRANSLATION_UNITS})

target_link_libraries(${PROJECT_NAME} ${MY_EXTERNAL_LIBS} ${CMAKE_DL_LIBS})