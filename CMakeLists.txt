
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

# FILE GROUP SRC_CMAKEGEN
set(SRC_CMAKEGEN
	"CMakeGen/CMakeLists.template"
)

source_group("CMakeGen" FILES ${SRC_CMAKEGEN})

# FILE GROUP SRC_INCLUDE
set(SRC_INCLUDE
	"include/peetcs.hpp"
	"include/archetype_pool.hpp"
	"include/query_value.hpp"
	"include/generic_container.hpp"
)

source_group("include" FILES ${SRC_INCLUDE})


# FILE GROUP SRC_SRC
set(SRC_SRC
	"src/archetype_pool.cpp"
	"src/query_value.cpp"
	"src/generic_container.cpp"
)

source_group("src" FILES ${SRC_SRC})


# FILE GROUP SRC_TESTS
set(SRC_TESTS
	"tests/unit_tests.hpp"
)

source_group("tests" FILES ${SRC_TESTS})


# LIBRARY GROUP PeetCS
set(SRC_LIB_PEETCS ${SRC_CMAKEGEN} ${SRC_INCLUDE} ${SRC_SRC} ${SRC_TESTS} )


# define the target
add_project(PEETCS
            STATIC_LIB
            SOURCES ${SRC_LIB_PEETCS}
            DEPENDS )

# FILE GROUP SRC_TESTS
set(SRC_SHARED
	"tests/shared.hpp"
	"src/shared.cpp"
)

source_group("Shared" FILES ${SRC_SHARED})

# define the target
add_project(SHARED
            SHARED_LIB
            SOURCES ${SRC_SHARED}
            DEPENDS )


# FILE GROUP SRC_PEETCS
set(SRC_MAIN
	"CMakeLists.txt"
	"main.cpp"
	"README.md"
)

source_group("Main" FILES ${SRC_MAIN})

# define the target
add_project(TESTER
            COMMAND
            SOURCES ${SRC_MAIN}
            DEPENDS PEETCS)

