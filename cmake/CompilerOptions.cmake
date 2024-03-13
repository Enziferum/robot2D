include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag description)
    message(STATUS "Checking flag: ${description}")
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

function(process_logging_compiler_flags)
    message("Check and enable supported c++ compiler flags:")
    message("-----------------------------------------------")
    if(MSVC)
        enable_cxx_compiler_flag_if_supported("/W3" "W3")
        # Show C4068 (unknown pragma) only on warning level 4
        enable_cxx_compiler_flag_if_supported("/w44068" "MSVC C4068")
        # Show C4200 (zero-sized array in struct/union) only on warning level 4
        enable_cxx_compiler_flag_if_supported("/w44200" "MSVC C4200")
    else()
        enable_cxx_compiler_flag_if_supported("-Wall" "-Wall")
        enable_cxx_compiler_flag_if_supported("-Wextra" "-Wextra")
        enable_cxx_compiler_flag_if_supported("-Wpedantic" "-Wpedantic")
        enable_cxx_compiler_flag_if_supported("-Wreorder" "-Wreorder")
    endif()
    message("-----------------------------------------------")
endfunction()
