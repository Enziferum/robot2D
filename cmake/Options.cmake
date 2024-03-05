# Some default variables which the user may change
option(CMAKE_BUILD_TYPE "Choose the type of build (Debug or Release)" Debug)
option(RB2D_BUILD_SHARED_LIBS "Whether to build shared libraries" OFF)
option(RB2D_BUILD_SANDBOX "Build Engine's sandbox submodule?" OFF)
option(RB2D_BUILD_TESTS "Build Engine's tests?" OFF)

# TODO(a.raag): Engine isn't ownself part
option(RB2D_BUILD_EDITOR "Build Editor?" OFF)
option(RB2D_INSTALL_CORE "Install Robot2D's core into system folders?" OFF)
option(RB2D_USE_CMAKE_VERBOSE "Use cmake verbose output" OFF)

macro(process_logging_options)
    include(CMakePrintHelpers)
    message("Robot2D Options: ")
    message("-----------------------------------------------")
    cmake_print_variables(RB2D_BUILD_TESTS)
    cmake_print_variables(RB2D_BUILD_SANDBOX)
    cmake_print_variables(RB2D_BUILD_EDITOR)
    cmake_print_variables(RB2D_INSTALL_CORE)
    message("-----------------------------------------------")
endmacro()
