# Some default variables which the user may change
option(CMAKE_BUILD_TYPE "Choose the type of build (Debug or Release)" Debug)
option(RB2D_USE_CMAKE_VERBOSE "Use cmake verbose output" OFF)
option(RB2D_BUILD_EDITOR "Build Editor?" OFF)


macro(process_logging_base_options)
    include(CMakePrintHelpers)
    message("Robot2D Base Options: ")
    message("-----------------------------------------------")
    cmake_print_variables(RB2D_USE_CMAKE_VERBOSE)
    cmake_print_variables(RB2D_BUILD_EDITOR)
    message("-----------------------------------------------")
endmacro()
