# Version and revision come from the root cmake/Version.cmake.
configure_file("${CMAKE_CURRENT_LIST_DIR}/BuildInfo.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.hpp.in" @ONLY)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/$<CONFIG>/editor/BuildInfo.hpp"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.hpp.in")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated/$<CONFIG>")
