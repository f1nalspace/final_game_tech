cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
#
# Project settings
#
# -----------------------------------------------------------------------------
project(FPL_ImGui)

set(MY_EXTERNAL_LIBS
        GL
        )

set(MY_HEADER_FILES
        ../../final_platform_layer.h
        )

set(MY_INCLUDE_DIRS
        dependencies/imgui/include/
        )

set(MY_SOURCE_FILES
        fpl_imgui.cpp
        dependencies/imgui/include/imgui/imgui.cpp
        dependencies/imgui/include/imgui/imgui_draw.cpp
        dependencies/imgui/include/imgui/imgui_demo.cpp
        )

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

add_executable(${PROJECT_NAME} ${MY_HEADER_FILES} ${MY_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${MY_EXTERNAL_LIBS} ${CMAKE_DL_LIBS})
