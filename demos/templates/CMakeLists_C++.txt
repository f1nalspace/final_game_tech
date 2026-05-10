cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------
project(FPL_Crackout CXX)

set(MY_CPP_STANDARD "cpp11")

set(MY_EXTERNAL_LIBS "")

set(MY_INCLUDE_DIRS
	"../../"
	"../additions/"
	"../dependencies/"
	)

set(MY_HEADER_FILES
	"../../final_platform_layer.h"
	"../additions/final_game.h"
	"../additions/final_gameplatform.h"
	)

set(MY_TRANSLATION_UNITS
	"fpl_crackout.cpp"
	)

set(MY_DEFINES "")

# -----------------------------------------------------------------------------
#
# Do not change the following lines
#
# -----------------------------------------------------------------------------

# Detect architecture by pointer size
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
	set(FPL_ARCH arm64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
	set(FPL_ARCH arm32)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(FPL_ARCH x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(FPL_ARCH x86)
else()
	set(FPL_ARCH unknown)
endif()

# Force English diagnostics
add_compile_options(-fdiagnostics-plain-output)
add_compile_options(-fdiagnostics-color=always)
add_compile_options(-fdiagnostics-format=text)
add_compile_options(-fdiagnostics-show-option)
set(ENV{LC_ALL} "C")
set(ENV{LANG} "C")

# Setup output path
set(FPL_ROOT_PATH_RELATIVE ../)
get_filename_component(FPL_ROOT_PATH ${FPL_ROOT_PATH_RELATIVE} ABSOLUTE)
set(FPL_EXECUTABLE_NAME ${PROJECT_NAME})
set(FPL_EXECUTABLE_PATH ${FPL_ROOT_PATH}/build/${PROJECT_NAME}/${CMAKE_SYSTEM_NAME}-${FPL_ARCH}-${CMAKE_BUILD_TYPE})

message(STATUS "\n")
message(STATUS "FPL CMake Infos: ${PROJECT_NAME}")
message(STATUS "---------------------------------------------------------------")
message(STATUS "Architecture: ${FPL_ARCH}")
message(STATUS "C++-Standard: ${MY_CPP_STANDARD}")
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${FPL_EXECUTABLE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${FPL_EXECUTABLE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${FPL_EXECUTABLE_PATH})

add_executable(${PROJECT_NAME} ${MY_TRANSLATION_UNITS})

# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MY_CPP_STANDARD MATCHES "cpp11")
	set(CMAKE_CXX_STANDARD 11)
elseif (MY_CPP_STANDARD MATCHES "cpp14")
	set(CMAKE_CXX_STANDARD 14)
elseif (MY_CPP_STANDARD MATCHES "cpp17")
	set(CMAKE_CXX_STANDARD 17)
elseif (MY_CPP_STANDARD MATCHES "cpp20")
	set(CMAKE_CXX_STANDARD 20)
elseif (MY_CPP_STANDARD MATCHES "cpp23")
	set(CMAKE_CXX_STANDARD 23)
else()
	set(CMAKE_CXX_STANDARD 98)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE ${MY_DEFINES})

target_include_directories(${PROJECT_NAME} PRIVATE ${MY_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${MY_EXTERNAL_LIBS} ${CMAKE_DL_LIBS})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)
