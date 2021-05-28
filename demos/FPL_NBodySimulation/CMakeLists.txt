cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------
project(FPL_NBodySimulation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
	)

set(MY_TRANSLATION_UNITS
	"fpl_nbodysimulation.cpp"
	"final_platform_layer.cpp"
	)

set(MY_DEFINES
	-DFPL_LOGGING
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
