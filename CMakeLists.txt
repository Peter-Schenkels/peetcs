
#header directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

message("Headers: ${CMAKE_CURRENT_SOURCE_DIR}")

cmake_minimum_required(VERSION 3.5)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CMake/AddProjectMacro/Macros.cmake)
###############################
# Add all sources and headers #
##############################






# FILE GROUP SRC_BABECS_-_NEW
set(SRC_BABECS_-_NEW
	"cmakegen.exe"
	"CMakeLists.txt"
	"ecs.hpp"
	"main.cpp"
)

source_group("BabECS - New" FILES ${SRC_BABECS_-_NEW})


# FILE GROUP SRC_CMAKEGEN
set(SRC_CMAKEGEN
	"CMakeGen/CMakeLists.template"
)

source_group("CMakeGen" FILES ${SRC_CMAKEGEN})


# LIBRARY GROUP BabECS - New
set(SRC_LIB_BABECS_-_NEW ${SRC_BABECS_-_NEW} ${SRC_CMAKEGEN} )


# define the target
add_project(BABECS_-_NEW
            COMMAND
            SOURCES ${SRC_LIB_BABECS_-_NEW}
            DEPENDS )


