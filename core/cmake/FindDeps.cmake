# - logging finding package
macro(logging_find_package PackageName MissingList)
    message(CHECK_START "Finding package ${PackageName}")
    find_package(${PackageName} QUIET)
    if(NOT ${PackageName}_FOUND)
        message(CHECK_FAIL "not found")
        list(APPEND ${MissingList} ${PackageName})
    else ()
        message(CHECK_PASS "found")
    endif ()
endmacro()

function(process_logging_core_deps)

    message(CHECK_START "Robot2D's Core deps:")
    message("-----------------------------------------------")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    unset(missingComponents)

    logging_find_package(glfw3 missingComponents)
    logging_find_package(spdlog missingComponents)	

    if(RB2D_OS_WINDOWS)
        if (RB2D_COMPILER_MSVC OR (RB2D_COMPILER_CLANG AND NOT MINGW))
            list(APPEND CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}/extlibs/libs-msvc-universal/x64")
        elseif (RB2D_COMPILER_GCC OR (RB2D_COMPILER_CLANG AND MINGW))
            list(APPEND CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}/extlibs/libs-mingw/x64")
        endif ()
        list(APPEND CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/extlibs/headers/freetype2")
    endif ()

    logging_find_package(Freetype missingComponents)

    list(POP_BACK CMAKE_MESSAGE_INDENT)
    if(missingComponents)
        message(CHECK_FAIL "missing components: ${missingComponents}")
        message("-----------------------------------------------")
        message(FATAL_ERROR "${missingComponents} aren't installed into system. \n"
                "Please use install_deps.py inside scripts folder. \n"
                "python/python3 install_deps.py. \n"
                "If install_deps.py don't support your system try install ${missingComponents} yourself.")
    else()
        message(CHECK_PASS "all components found")
        #todo(a.raag): FreeType Correcly here or later ???
        include_directories(${FREETYPE_INCLUDE_DIRS})
    endif()
    message("-----------------------------------------------")

    # OpenGL
    if(WIN32)
        set(LIBS opengl32)
    elseif(APPLE)
        include_directories(/System/Library/Frameworks)
        find_library(OpenGL_LIBRARY OpenGL)
        mark_as_advanced(OpenGL_LIBRARY)
        set(APPLE_LIBS ${OpenGL_LIBRARY} )
        set(APPLE_LIBS ${APPLE_LIBS})
        set(LIBS ${APPLE_LIBS})
    elseif(UNIX)
        find_package(OpenGL REQUIRED)
        set(LIBS OpenGL::GL)
    endif()
endfunction()
