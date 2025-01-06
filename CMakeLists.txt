
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






# FILE GROUP SRC_PEETCS
set(SRC_PEETCS
	"cmakegen.exe"
	"CMakeLists.txt"
	"main.cpp"
	"README.md"
)

source_group("PeetCS" FILES ${SRC_PEETCS})


# FILE GROUP SRC_CMAKEGEN
set(SRC_CMAKEGEN
	"CMakeGen/CMakeLists.template"
)

source_group("CMakeGen" FILES ${SRC_CMAKEGEN})


# FILE GROUP SRC_INCLUDE
set(SRC_INCLUDE
	"include/archetype_block.hpp"
	"include/archetype_id.hpp"
	"include/archetype_pool.hpp"
	"include/peetcs_types.hpp"
	"include/query_value.hpp"
	"include/storage.hpp"
)

source_group("include" FILES ${SRC_INCLUDE})


# FILE GROUP SRC_SRC
set(SRC_SRC
	"src/archetype_block.cpp"
	"src/archetype_id.cpp"
	"src/archetype_pool.cpp"
	"src/query_value.cpp"
	"src/storage.cpp"
)

source_group("src" FILES ${SRC_SRC})


# LIBRARY GROUP PeetCS
set(SRC_LIB_PEETCS ${SRC_PEETCS} ${SRC_CMAKEGEN} ${SRC_INCLUDE} ${SRC_SRC} )


# define the target
add_project(PEETCS
            COMMAND
            SOURCES ${SRC_LIB_PEETCS}
            DEPENDS )


