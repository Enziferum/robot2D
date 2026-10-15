# Version numbers have one source: the root project(... VERSION ...).
set(RB2D_VERSION_PRERELEASE "dev" CACHE STRING "Prerelease identifier; empty for a stable release")
set(RB2D_EDITOR_BUILD_ID "" CACHE STRING "Build identifier override; empty uses the Git revision")
if(NOT RB2D_VERSION_PRERELEASE STREQUAL "" AND
        NOT RB2D_VERSION_PRERELEASE MATCHES "^[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)*$")
    message(FATAL_ERROR "RB2D_VERSION_PRERELEASE must contain dot-separated letters, digits or '-'")
endif()
foreach(part IN LISTS RB2D_VERSION_PRERELEASE)
    # Numeric prerelease components must not contain leading zeroes.
    string(REPLACE "." ";" identifiers "${part}")
    foreach(identifier IN LISTS identifiers)
        if(identifier MATCHES "^0[0-9]+$")
            message(FATAL_ERROR "Numeric prerelease identifiers must not have leading zeroes")
        endif()
    endforeach()
endforeach()

set(RB2D_GIT_SHA "unknown")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --show-toplevel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE git_root
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE git_result)
    # An unpacked source archive nested in another repository must not borrow its SHA.
    if(git_result EQUAL 0)
        file(REAL_PATH "${git_root}" git_root)
        file(REAL_PATH "${CMAKE_SOURCE_DIR}" source_root)
        if(git_root STREQUAL source_root)
            execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --verify HEAD
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE git_sha
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE git_result)
            if(git_result EQUAL 0 AND git_sha MATCHES "^[0-9a-fA-F]+$")
                set(RB2D_GIT_SHA "${git_sha}")
            endif()
            execute_process(COMMAND "${GIT_EXECUTABLE}" symbolic-ref -q HEAD
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE git_branch
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            foreach(ref IN ITEMS HEAD packed-refs "${git_branch}")
                if(ref STREQUAL "")
                    continue()
                endif()
                execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --git-path "${ref}"
                        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE git_path
                        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE git_result)
                if(git_result EQUAL 0 AND NOT git_path STREQUAL "")
                    get_filename_component(git_path "${git_path}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
                    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${git_path}")
                endif()
            endforeach()
        endif()
    endif()
endif()

set(RB2D_BUILD_ID "${RB2D_EDITOR_BUILD_ID}")
set(RB2D_VERSION "${CMAKE_PROJECT_VERSION}")
if(NOT RB2D_VERSION_PRERELEASE STREQUAL "")
    string(APPEND RB2D_VERSION "-${RB2D_VERSION_PRERELEASE}")
endif()
set(RB2D_VERSION_FULL "${RB2D_VERSION}")
if(RB2D_GIT_SHA STREQUAL "unknown")
    set(default_build_id "local")
    set(version_metadata "local")
else()
    string(SUBSTRING "${RB2D_GIT_SHA}" 0 12 default_build_id)
    set(version_metadata "g${default_build_id}")
endif()
if(RB2D_BUILD_ID STREQUAL "")
    set(RB2D_BUILD_ID "${default_build_id}")
endif()
if(NOT RB2D_BUILD_ID MATCHES "^[A-Za-z0-9_.+-]+$")
    message(FATAL_ERROR "RB2D_EDITOR_BUILD_ID must contain only letters, digits, '.', '_', '+' or '-'")
endif()
if(NOT RB2D_VERSION_PRERELEASE STREQUAL "")
    string(APPEND RB2D_VERSION_FULL "+${version_metadata}")
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/../core/include/robot2D/Version.hpp.in"
        "${CMAKE_BINARY_DIR}/generated/include/robot2D/Version.hpp" @ONLY)
message(STATUS "robot2D version: ${RB2D_VERSION_FULL}, build ${RB2D_BUILD_ID}")
