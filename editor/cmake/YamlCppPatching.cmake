add_subdirectory(extlibs/yaml-cpp)

# The pinned yaml-cpp revision uses uint16_t/uint32_t in emitterutils.cpp
# without including <cstdint>. GCC 16 no longer supplies them transitively.
# Keep the submodule unchanged; supply the missing header for this source only.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/extlibs/yaml-cpp/src/emitterutils.cpp"
            DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/extlibs/yaml-cpp"
            APPEND PROPERTY COMPILE_OPTIONS -include cstdint)
endif()
