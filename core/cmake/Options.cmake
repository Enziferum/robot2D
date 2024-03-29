option(RB2D_BUILD_SHARED_LIBS "Whether to build core's shared libraries" OFF)
option(RB2D_BUILD_CORE_SANDBOX "Build Core's sandbox submodule?" OFF)
option(RB2D_BUILD_CORE_TESTS "Build Core's tests?" OFF)
option(RB2D_INSTALL_CORE "Install Robot2D's core into system folders?" OFF)

macro(process_logging_options)
    include(CMakePrintHelpers)
    message("Robot2D-Core Options: ")
    message("-----------------------------------------------")
    cmake_print_variables(RB2D_BUILD_CORE_TESTS)
    cmake_print_variables(RB2D_BUILD_CORE_SANDBOX)
    cmake_print_variables(RB2D_INSTALL_CORE)
    cmake_print_variables(RB2D_BUILD_SHARED_LIBS)
    message("-----------------------------------------------")
endmacro()