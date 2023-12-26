include(CMakeParseArguments)
include(copy_to_runtime_path)

find_package(Mono)

if (NOT MONO_FOUND)
    message(FATAL_ERROR "Mono is required for this CMake script. Please install it.")
endif ()

if (WIN32)
    find_file(MONO_DLL_PATH NAMES mono-2.0.dll mono-2.0-sgen.dll PATHS ${MONO_BINARY_PATH})
    copy_files_to_runtime_path(FILES ${MONO_DLL_PATH})
endif ()

function(add_mono_assembly)
    cmake_parse_arguments(
        MONO_ASSEMBLY_PARSED_ARGS
        ""
        "TARGET;DESTINATION;TYPE"
        "SOURCES;ADDITIONAL_ARGS"
        ${ARGN}
    )

    if (NOT MONO_ASSEMBLY_PARSED_ARGS_TYPE)
        set(MONO_ASSEMBLY_PARSED_ARGS_TYPE "library")
        message("No type provided for ${MONO_ASSEMBLY_PARSED_ARGS_TARGET}. Assuming library.")
    endif ()

    set(_FILE_EXTENSION "")
    if (MONO_ASSEMBLY_PARSED_ARGS_TYPE STREQUAL "library")
        set(_FILE_EXTENSION ".dll")
    elseif (MONO_ASSEMBLY_PARSED_ARGS_TYPE STREQUAL "exe")
        set(_FILE_EXTENSION ".exe")
    else ()
        message(FATAL_ERROR "Type must be either exe or library.")
    endif ()

    set(FULL_PATH_SOURCES ${MONO_ASSEMBLY_PARSED_ARGS_SOURCES})

    if (NOT MONO_ASSEMBLY_PARSED_ARGS_DESTINATION)
        set(MONO_ASSEMBLY_PARSED_ARGS_DESTINATION ${CMAKE_BINARY_DIR})
    endif ()

    file(MAKE_DIRECTORY ${MONO_ASSEMBLY_PARSED_ARGS_DESTINATION})

    #Dotnet package config disabled, since it only works properly on 32-bit.
    #get_mono_pkg_config("dotnet" MONO_DOTNET_PKG_CONFIG)
    add_custom_target(
        ${MONO_ASSEMBLY_PARSED_ARGS_TARGET} ALL
        ${MONO_MCS_EXECUTABLE} "-t:${MONO_ASSEMBLY_PARSED_ARGS_TYPE}" ${MONO_ASSEMBLY_PARSED_ARGS_ADDITIONAL_ARGS} ${FULL_PATH_SOURCES} "-out:${MONO_ASSEMBLY_PARSED_ARGS_TARGET}${_FILE_EXTENSION}"
        WORKING_DIRECTORY "${MONO_ASSEMBLY_PARSED_ARGS_DESTINATION}"
        COMMENT "Building Mono Library ${MONO_ASSEMBLY_PARSED_ARGS_TARGET}"
        SOURCES ${FULL_PATH_SOURCES}
    )

endfunction()

function(copy_mono_runtimes_to_runtime_path)
    message(STATUS "Copying Mono runtimes to the runtime path.")
    copy_folder_to_runtime_path(
        PATH ${MONO_LIBRARY_PATH}/mono
    )
endfunction()

function(get_mono_pkg_config package output)
    execute_process(COMMAND "${MONO_PKG_CONFIG_EXECUTABLE}" "--libs" ${package} OUTPUT_VARIABLE _OUTPUT OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" _OUTPUT "${_OUTPUT}")
    set(${output} ${_OUTPUT} PARENT_SCOPE)
endfunction()
