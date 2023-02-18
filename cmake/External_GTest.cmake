include(ExternalProject)

ExternalProject_Add(gtest_ext
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/gmock-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/gmock-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/gtest-build"
        CMAKE_ARGS "${GTEST_CMAKE_ARGS}"
        "-DCMAKE_BUILD_TYPE=Release"
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        )
set(GTEST_INCLUDE_DIRS
        "${CMAKE_BINARY_DIR}/third-party/gmock-src/googletest/include"
        "${CMAKE_BINARY_DIR}/third-party/gmock-src/googlemock/include"
        )
link_directories("${CMAKE_BINARY_DIR}/third-party/gmock-build/lib")

enable_testing()

find_package(Threads REQUIRED)

function(cxx_test name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} ${ARGN} gtest ${CMAKE_THREAD_LIBS_INIT})
    set_property(TARGET ${name} APPEND PROPERTY INCLUDE_DIRECTORIES
            ${GTEST_INCLUDE_DIRS}
            )
    add_dependencies(${name} gtest_ext)
    add_test(NAME ${name}
            COMMAND ${name} "--gtest_break_on_failure")
endfunction()