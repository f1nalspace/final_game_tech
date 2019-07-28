cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------
project(FPL_Audio)

set(CMAKE_C_FLAGS "-std=c99")

set(MY_EXTERNAL_LIBS m)

set(MY_INCLUDE_DIRS ../../ ../additions/ ../dependencies/)

set(MY_HEADER_FILES ../../final_platform_layer.h ../additions/final_audiosystem.h ../additions/final_waveloader.h)

set(MY_SOURCE_FILES fpl_audio.c)

# Add all defines here, so we can fully debug inside the header files
add_definitions(-DFPL_IMPLEMENTATION -DFPL_LOGGING)

# -----------------------------------------------------------------------------
#
# Do not change the following lines
#
# -----------------------------------------------------------------------------

set(MY_EXECUTABLE_NAME ${PROJECT_NAME})
set(MY_EXECUTABLE_PATH ../../${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}/)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MY_EXECUTABLE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MY_EXECUTABLE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MY_EXECUTABLE_PATH})

include_directories(../../ ${MY_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${MY_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${MY_EXTERNAL_LIBS} ${CMAKE_DL_LIBS})