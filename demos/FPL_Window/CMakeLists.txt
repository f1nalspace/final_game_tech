cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------

project(FPL_Window C)

set(MY_COMPILER "c99")

set(MY_EXTERNAL_LIBS "")

set(MY_INCLUDE_DIRS
	"../../"
	)

set(MY_HEADER_FILES
	"../../final_platform_layer.h"
	)

set(MY_TRANSLATION_UNITS
	"fpl_window.c"
	)

set(MY_DEFINES
	-DFPL_IMPLEMENTATION
	)

# -----------------------------------------------------------------------------
#
# Do not change the following lines
#
# -----------------------------------------------------------------------------

# Architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
	set(THIS_ARCH arm64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
	set(THIS_ARCH arm32)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(THIS_ARCH x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(THIS_ARCH x86)
else()
	set(THIS_ARCH unknown)
endif()

# Output path setup
set(THIS_ROOT_PATH_RELATIVE ../)
get_filename_component(THIS_ROOT_PATH ${THIS_ROOT_PATH_RELATIVE} ABSOLUTE)

set(THIS_EXECUTABLE_NAME ${PROJECT_NAME})
set(THIS_EXECUTABLE_PATH ${THIS_ROOT_PATH}/build/${PROJECT_NAME}/${CMAKE_SYSTEM_NAME}-${THIS_ARCH}-${CMAKE_BUILD_TYPE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${THIS_EXECUTABLE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${THIS_EXECUTABLE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${THIS_EXECUTABLE_PATH})

# Diagnostics as english
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fdiagnostics-plain-output)
endif()
add_compile_options(-fdiagnostics-color=always)
add_compile_options(-fdiagnostics-format=text)
add_compile_options(-fdiagnostics-show-option)
set(ENV{LC_ALL} "C")
set(ENV{LANG} "C")

# Build info
message(STATUS "\n")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "---------------------------------------------------------------")
message(STATUS "Architecture: ${THIS_ARCH}")
message(STATUS "Compiler standard: ${MY_COMPILER}")
message(STATUS "External libraries: ${MY_EXTERNAL_LIBS}")
message(STATUS "Include directories: ${MY_INCLUDE_DIRS}")
message(STATUS "Header files: ${MY_HEADER_FILES}")
message(STATUS "Translation units: ${MY_TRANSLATION_UNITS}")
message(STATUS "Defines: ${MY_DEFINES}")
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Root dir: ${THIS_ROOT_PATH}")
message(STATUS "Executable path: ${THIS_EXECUTABLE_PATH}")
message(STATUS "Executable name: ${THIS_EXECUTABLE_NAME}")
message(STATUS "---------------------------------------------------------------\n")

# Project executable (must come first before anything else)
add_executable(${PROJECT_NAME} ${MY_TRANSLATION_UNITS})

# Compiler detection
if(DEFINED MY_COMPILER AND NOT "${MY_COMPILER}" STREQUAL "")
	if(MY_COMPILER MATCHES "^c[0-9]+$")
		enable_language(C)
		set(CMAKE_C_STANDARD_REQUIRED ON)
		set(CMAKE_C_EXTENSIONS OFF)

		if(MY_COMPILER STREQUAL "c99")
			set(CMAKE_C_STANDARD 99)
			target_compile_features(${PROJECT_NAME} PUBLIC c_std_99)
		elseif(MY_COMPILER STREQUAL "c11")
			set(CMAKE_C_STANDARD 11)
			target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)
		elseif(MY_COMPILER STREQUAL "c17")
			set(CMAKE_C_STANDARD 17)
			target_compile_features(${PROJECT_NAME} PUBLIC c_std_17)
		else()
			message(FATAL_ERROR "Unsupported C Compiler: ${MY_COMPILER}")
		endif()
	elseif(MY_COMPILER MATCHES "^cpp[0-9]+$")
		enable_language(CXX)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)

		if(MY_COMPILER STREQUAL "cpp11")
			set(CMAKE_CXX_STANDARD 11)
		elseif(MY_COMPILER STREQUAL "cpp14")
			set(CMAKE_CXX_STANDARD 14)
		elseif(MY_COMPILER STREQUAL "cpp17")
			set(CMAKE_CXX_STANDARD 17)
		elseif(MY_COMPILER STREQUAL "cpp20")
			set(CMAKE_CXX_STANDARD 20)
		elseif(MY_COMPILER STREQUAL "cpp23")
			set(CMAKE_CXX_STANDARD 23)
		else()
			message(FATAL_ERROR "Unsupported C++ Compiler: ${MY_COMPILER}")
		endif()
	else()
		message(FATAL_ERROR "Unknown Compiler: ${MY_COMPILER}")
	endif()
else()
	message(FATAL_ERROR "Compiler is not set, set MY_COMPILER to a correct c or cpp standard!")
endif()

# Build target
target_compile_definitions(${PROJECT_NAME} PRIVATE ${MY_DEFINES})

target_include_directories(${PROJECT_NAME} PRIVATE ${MY_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${MY_EXTERNAL_LIBS} ${CMAKE_DL_LIBS})

# Post-build copy of resource folder
if(DEFINED MY_RESOURCE_FOLDER AND NOT "${MY_RESOURCE_FOLDER}" STREQUAL "")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_SOURCE_DIR}/${MY_RESOURCE_FOLDER}
			$<TARGET_FILE_DIR:${PROJECT_NAME}>/${MY_RESOURCE_FOLDER}
	)
endif()
