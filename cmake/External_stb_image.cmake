include (ExternalProject)

ExternalProject_Add(stb_image_ext
        GIT_REPOSITORY https://github.com/Enziferum/stb_image.git
        BINARY_DIR "${CMAKE_BINARY_DIR}/third-party/stb_image-build"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third-party/stb_image-src"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/third-party/stb_image-build"
        CMAKE_ARGS "${stb_image_CMAKE_ARGS}"
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/stb_image-build
        INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install
        UPDATE_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . && ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE}
        )
set(STB_IMAGE_INCLUDE_DIRS
        "${CMAKE_BINARY_DIR}/third-party/stb_image-build/incl"
        CACHE PATH "stb_image include directory"
        )

link_directories("${CMAKE_BINARY_DIR}/third-party/stb_image-build/dist/lib")
set(STB_IMAGE_LIBRARIES stb_image CACHE STRING "stb_image library")
