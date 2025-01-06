# AddProjectMacro
 CMake macro for easy/lazy project library/executable/target creation with linking.


# Example

```cmake
include("AddProjectMacro/Macros.cmake")


...stuff


# FILE GROUP SRC_CONTEMPLATOR
set(SRC_A
	"b/a.cpp"
	"b/a.hpp"
)


set(SRC_B
	"b/b.cpp"
	"b/b.hpp"
)

# Creates a group "A" in Visual Studio e.g.
source_group("A" FILES ${SRC_A})
# etc...
source_group("B" FILES ${SRC_B})

set(AB_SRC ${SRC_A} ${SRC_B} )


# define the target
add_project(CONTEMPLATOR                           #  <---- Target name
            STATIC_LIB                             #  <---- Target Type
            SOURCES ${AB_SRC}                      #  <---- Sources
            DEPENDS nlohmann_json::nlohmann_json ) #  <---- Target dependencies

```

# Library types
* STATIC_LIB -> Static library
* SHARED_LIB -> Creates a DLL
* EXE        -> Creates an executable
* EXAMPLE    -> Same as EXE
* SHADERS    -> (Ignore this for now)
* COMMAND    -> Creates an executable and copies the executable to the src folder.



Please check out Macros.cmake to see what's REALLY happening.

