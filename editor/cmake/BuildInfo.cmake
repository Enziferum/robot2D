set(RB2D_EDITOR_BUILD_ID "" CACHE STRING "Editor build identifier (empty uses the Git revision)")

set(RB2D_BUILD_ID "${RB2D_EDITOR_BUILD_ID}")
if(RB2D_BUILD_ID STREQUAL "")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                OUTPUT_VARIABLE RB2D_BUILD_ID
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)

        # Refresh the revision when HEAD changes, including worktree checkouts.
        foreach(RB2D_GIT_REF HEAD packed-refs)
            execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --git-path "${RB2D_GIT_REF}"
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                    OUTPUT_VARIABLE RB2D_GIT_PATH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            if(RB2D_GIT_PATH)
                get_filename_component(RB2D_GIT_PATH "${RB2D_GIT_PATH}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
                set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${RB2D_GIT_PATH}")
            endif()
        endforeach()
        execute_process(COMMAND "${GIT_EXECUTABLE}" symbolic-ref -q HEAD
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                OUTPUT_VARIABLE RB2D_GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        if(RB2D_GIT_BRANCH)
            execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --git-path "${RB2D_GIT_BRANCH}"
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                    OUTPUT_VARIABLE RB2D_GIT_PATH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            get_filename_component(RB2D_GIT_PATH "${RB2D_GIT_PATH}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
            set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${RB2D_GIT_PATH}")
        endif()
    endif()
    if(RB2D_BUILD_ID STREQUAL "")
        set(RB2D_BUILD_ID "local")
    endif()
endif()

if(NOT RB2D_BUILD_ID MATCHES "^[A-Za-z0-9_.+-]+$")
    message(FATAL_ERROR "RB2D_EDITOR_BUILD_ID must contain only letters, digits, '.', '_', '+' or '-'")
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/BuildInfo.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.hpp.in" @ONLY)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/$<CONFIG>/editor/BuildInfo.hpp"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.hpp.in")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated/$<CONFIG>")
