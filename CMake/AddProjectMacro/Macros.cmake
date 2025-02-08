macro(add_project target)
    # Parse arguments with "COMMAND" added to handle command-line applications
    cmake_parse_arguments(THIS "STATIC_LIB;SHARED_LIB;GUI_APP;EXAMPLE;SHADERS;COMMAND" "NAME" "SOURCES;BUNDLE_RESOURCES;DEPENDS" ${ARGN})

    message("Adding target: " ${target})

    # Group source files
    source_group("" FILES ${THIS_SOURCES})

    # Gather input sources and bundle resources
    set(target_input ${THIS_SOURCES})
    if(THIS_BUNDLE_RESOURCES)
        set(target_input ${target_input} ${THIS_BUNDLE_RESOURCES})
    endif()

		# Initialize flags
	set(HAS_SOURCE_FILES FALSE)
	set(HAS_HEADER_FILES FALSE)

	# Loop through the file list
	foreach(FILE ${target_input})
		get_filename_component(EXT ${FILE} EXT)
		if(EXT STREQUAL ".cpp" OR EXT STREQUAL ".c")
			set(HAS_SOURCE_FILES TRUE)
		elseif(EXT STREQUAL ".h" OR EXT STREQUAL ".hpp")
			set(HAS_HEADER_FILES TRUE)
		endif()
	endforeach()

    # Create the target based on type
	if(NOT HAS_SOURCE_FILES AND NOT HAS_HEADER_FILES)
		add_library(${target} OBJECT ${target_input})
    elseif(THIS_STATIC_LIB)
        add_library(${target} STATIC ${target_input})
	elseif(THIS_SHARED_LIB)
        add_library(${target} SHARED ${target_input})
        add_compile_definitions(${target}_LIBRARY_EXPORTS)
    elseif(THIS_GUI_APP OR THIS_EXAMPLE OR THIS_COMMAND)
        add_executable(${target} ${target_input} ${GLAD_GL})
    elseif(THIS_SHADERS)
        add_custom_target(${target} SOURCES ${target_input})
    endif()

    # Post-build processing for non-shader targets
    if(NOT THIS_SHADERS)
        if(THIS_GUI_APP OR THIS_EXAMPLE OR THIS_STATIC_LIB OR THIS_SHARED_LIB)
            # Add vendor includes as SYSTEM to suppress warnings
            target_include_directories(${target} SYSTEM PRIVATE ${VENDOR_INCLUDE_DIR})
        endif()

        # Static standard library linking (MSVC-specific)
        if(USE_STATIC_STD_LIBS)
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()

        # Set target properties for debugging and IDE organization
        set_target_properties(${target} PROPERTIES DEBUG_POSTFIX -d)
        set_target_properties(${target} PROPERTIES FOLDER "Projects")
        set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

        # Link dependencies if specified
        if(THIS_DEPENDS)
            target_link_libraries(${target} PRIVATE ${THIS_DEPENDS})
        endif()

        # Apply optimization flags based on build type and compiler
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
                target_compile_options(${target} PRIVATE -O3 -DNDEBUG)
            elseif(MSVC)
                target_compile_options(${target} PRIVATE /DNDEBUG)
            endif()
        endif()

        # Suppress warnings for vendor target if necessary
        if(${target} STREQUAL Vendor OR THIS_EXAMPLE)
            message("IS VENDOR")
            if(MSVC)
                target_compile_options(${target} PRIVATE /W0)
            else()
                target_compile_options(${target} PRIVATE -w)
            endif()
        endif()

        # Copy the executable or library to the source directory for command-line apps or GUI apps
        if(THIS_COMMAND AND CMAKE_BUILD_TYPE STREQUAL "Release")
            copy_exe_to_source(${target})
        endif()
    endif()
endmacro()


macro(copy_exe_to_source target)
    # Ensure the target exists
    if (TARGET ${target})
        # Get the current source directory (where CMakeLists.txt is located)
        set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

        # Get the target type to ensure it is an executable
        get_target_property(target_type ${target} TYPE)

        if (target_type STREQUAL "EXECUTABLE" OR target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY")
            # Safely retrieve the target file name using generator expressions
            add_custom_command(
                TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:${target}>" > file.txt # Debugging step to see the file path
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${target}>      # Use generator expression for the target's file
                    ${SOURCE_DIR}/$<LOWER_CASE:$<TARGET_FILE_NAME:${target}>>  # Use lowercase file name
                COMMENT "Copying ${target} to the source directory: ${SOURCE_DIR}"
            )
        else()
            message(WARNING "Target ${target} is not an executable or library, skipping copy.")
        endif()
    else()
        message(WARNING "Target ${target} does not exist.")
    endif()
endmacro()
