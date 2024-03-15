# detect the OS
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(RB2D_OS_WINDOWS 1)

    # don't use the OpenGL ES implementation on Windows
    set(OPENGL_ES 0)

    # detect the architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_64BITS 1)
    else()
        message(FATAL_ERROR "Unsupported architecture")
        return()
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(RB2D_OS_UNIX 1)

    if(ANDROID)
        set(RB2D_OS_ANDROID 1)

        # use the OpenGL ES implementation on Android
        set(OPENGL_ES 1)
    else()
        set(RB2D_OS_LINUX 1)

        # don't use the OpenGL ES implementation on Linux
        set(OPENGL_ES 0)
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(RB2D_OS_MACOS 1)

    # don't use the OpenGL ES implementation on macOS
    set(OPENGL_ES 0)
elseif(${CYGWIN})
    message(FATAL_ERROR "Unfortunately Robot2D doesn't support Cygwin's 'hybrid' status between both Windows and Linux derivatives.\nIf you insist on using the GCC, please use a standalone build of MinGW without the Cygwin environment instead.")
else()
    message(FATAL_ERROR "Unsupported operating system or environment")
    return()
endif()

# detect the compiler
# Note: The detection is order is important because:
# - Visual Studio can both use MSVC and Clang
# - GNUCXX can still be set on macOS when using Clang
if(MSVC)
    set(RB2D_COMPILER_MSVC 1)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(RB2D_COMPILER_CLANG_CL 1)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(RB2D_COMPILER_CLANG 1)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(RB2D_COMPILER_GCC 1)

    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "--version" OUTPUT_VARIABLE GCC_COMPILER_VERSION)
    string(REGEX MATCHALL ".*(tdm[64]*-[1-9]).*" RB2D_COMPILER_GCC_TDM "${GCC_COMPILER_VERSION}")
else()
    message(WARNING "Unrecognized compiler: ${CMAKE_CXX_COMPILER_ID}. Use at your own risk.")
endif()