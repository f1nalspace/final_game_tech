cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(FPL_OpenGL)

# Defines
set(MY_DEFINES
	-DFPL_IMPLEMENTATION
	-DFPL_LOGGING
)

# Include directories
set(MY_INCLUDE_DIRS
	"../../"
	"../additions/"
)

# Translation units
set(MY_TRANSLATION_UNITS
	"fpl_opengl.c"
)

# C/C++ Standard
set(CMAKE_C_STANDARD 99)

# External libs
find_package(OpenGL REQUIRED)
set(MY_EXTERNAL_LIBS ${OPENGL_LIBRARY})

# ---------------------------------
# ! Do not change the lines below !
# ---------------------------------

add_definitions(${MY_DEFINES})

include_directories(${MY_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${MY_TRANSLATION_UNITS})

target_link_libraries(${PROJECT_NAME} ${MY_EXTERNAL_LIBS} ${CMAKE_DL_LIBS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MY_TRANSLATION_UNITS})

install(TARGETS ${PROJECT_NAME} DESTINATION ./)