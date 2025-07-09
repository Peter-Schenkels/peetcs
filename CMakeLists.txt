
#header directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

message("Headers: ${CMAKE_CURRENT_SOURCE_DIR}")

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++20" )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_BUILD_TYPE "Release" CACHE
      STRING "Choose the type of build.")

include(CMake/AddProjectMacro/Macros.cmake)

# Add JSON libraries
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)


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
    "include/utilities.hpp"
)

# FILE GROUP SRC_INCLUDE_PIPO_RASTERIZER
set(SRC_INCLUDE_PIPO_RASTERIZER
	"include/pipo/rasterizer.hpp"
    "include/pipo/imgui.hpp"
)

# FILE GROUP SRC_SRC
set(SRC_SRC_PIPO
	"src/pipo/rasterizer.cpp"
    "src/pipo/imgui.cpp"
)


source_group("include" FILES ${SRC_INCLUDE})

project(Peetcs)


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
            DEPENDS nlohmann_json::nlohmann_json)

# FILE GROUP SRC_PEETCS
set(SRC_MAIN
	"CMakeLists.txt"
	"main.cpp"
	"README.md"
)

# GLFW include and lib paths
set(GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/glfw/include)
include_directories(${GLFW_INCLUDE_DIR})

# Determine correct GLFW lib folder
if (MSVC)
    # Detect toolset version (e.g. 1920+ is VS2019, 1930+ is VS2022)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.30")
        set(GLFW_LIB_DIR ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2022)
    elseif (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.20")
        set(GLFW_LIB_DIR ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2019)
    else()
        message(FATAL_ERROR "Unsupported MSVC version: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
    set(GLFW_LIB glfw3.lib)
else()
    # Assume MinGW if not MSVC
    set(GLFW_LIB_DIR ${CMAKE_SOURCE_DIR}/external/glfw/lib-mingw)
    set(GLFW_LIB libglfw3.a)
endif()

link_directories(${GLFW_LIB_DIR})

# Add path to GLEW include files
include_directories(${CMAKE_SOURCE_DIR}/external/glew/include)
# Add imgui
add_subdirectory(${CMAKE_SOURCE_DIR}/external/imgui)

include_directories(external/glm external/stb external/obj_loader external/imgui)

# Add path to GLEW libraries
link_directories(${CMAKE_SOURCE_DIR}/external/glew/lib/Release/x64)

set(SRC_LIB_PIPO_RASTERIZER ${SRC_INCLUDE_PIPO_RASTERIZER} ${SRC_SRC_PIPO})


# define the target
add_project(PIPO
            STATIC_LIB 
            SOURCES ${SRC_LIB_PIPO_RASTERIZER}
            DEPENDS PEETCS ${GLFW_LIB} glew32s opengl32 imgui nlohmann_json::nlohmann_json)

			
target_include_directories(PIPO PRIVATE external/glm external/stb external/obj_loader)


source_group("Main" FILES ${SRC_MAIN})


# FILE GROUP SRC_TESTS
set(SRC_PHESYCS
    "include/dll_macros.hpp"
	"tests/shared.hpp"
	"src/phesycs/shared.cpp"
	"src/phesycs/phesycs.cpp"
	"include/phesycs/phesycs.hpp"
)

source_group("Phesycs" FILES ${SRC_PHESYCS})

# define the target
add_project(PHESYCS
            SHARED_LIB
            SOURCES ${SRC_PHESYCS}
            DEPENDS PEETCS PIPO nlohmann_json::nlohmann_json)

# define the target
add_project(TESTER
            GUI_APP 
            SOURCES ${SRC_MAIN}
            DEPENDS PEETCS PIPO PHESYCS nlohmann_json::nlohmann_json)