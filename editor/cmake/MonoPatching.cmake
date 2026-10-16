include(CMakeParseArguments)
include(copy_to_runtime_path)

include("${CMAKE_CURRENT_LIST_DIR}/MonoAssembly.cmake")

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

find_package(Mono)

if (NOT MONO_FOUND)
    message(FATAL_ERROR "Mono is required for this CMake script. Please install it.")
endif ()

set(INTERNAL_MONO_ASSEMBLY_DIR "${MONO_ASSEMBLY_PATH}"
        CACHE PATH "Path to the Mono assembly dir (mono/lib). May be a relative path.")
set(INTERNAL_MONO_CONFIG_DIR "${MONO_CONFIG_PATH}"
        CACHE PATH "Path to the Mono config dir (mono/etc). May be a relative path.")

configure_file(src/scripting/mono_build_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/mono_build_config.h @ONLY)
