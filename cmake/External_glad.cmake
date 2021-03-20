include (ExternalProject)

ExternalProject_Add(glad_ext
        GIT_REPOSITORY https://github.com/Enziferum/glad.git
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/glad-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/glad-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/glad-build"
        CMAKE_ARGS "${glad_CMAKE_ARGS}"
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/glad-build
        INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install
        UPDATE_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . && ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE}
        )
set(GLAD_INCLUDE_DIRS
        "${CMAKE_BINARY_DIR}/third-party/glad-build/incl"
        CACHE PATH "Glad include directory"
        )

link_directories("${CMAKE_BINARY_DIR}/third-party/glad-build/lib")
set(GLAD_LIBRARIES glad CACHE PATH "glad library")
