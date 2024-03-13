include(${CMAKE_SOURCE_DIR}/cmake/Functions.cmake)
include(FindDeps)
include(Options)
# - brief processing core
process_logging_options()
process_logging_core_deps()

add_subdirectory(src)
add_library(${PROJECT_NAME} ${ENGINE_INCLUDES} ${ENGINE_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        INTERFACE
        "$<INSTALL_INTERFACE:include>")

if (RB2D_OS_WINDOWS)
    if(RB2D_COMPILER_MSVC)
      GroupSourcesByFolder(${PROJECT_NAME})
    endif ()
endif ()

set(LIBS ${LIBS} glfw spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE Freetype::Freetype)

if(RB2D_INSTALL_CORE)
    include(cmake/InstallCore.cmake)
endif ()

