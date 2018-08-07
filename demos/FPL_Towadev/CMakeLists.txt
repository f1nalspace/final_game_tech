cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------
project(FPL_Towadev)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MY_EXTERNAL_LIBS "")

set(MY_INCLUDE_DIRS ../dependencies/ ../additions/)

set(MY_HEADER_FILES ../../final_platform_layer.h)

set(MY_SOURCE_FILES fpl_towadev.cpp)

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

add_custom_command(TARGET FPL_Towadev POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:FPL_Towadev>/data)
