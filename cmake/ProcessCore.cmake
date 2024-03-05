include(Diagnostics)
include(Options)
include(CompilerOptions)
include(FindCoreDeps)

# - brief processing core
print_diagnostic()
process_logging_options()
process_logging_compiler_flags()
process_logging_core_deps()

# use spdlog for some time
add_subdirectory(engine)

add_library(${PROJECT_NAME} ${ENGINE_SRC})
set(LIBS ${LIBS} ${FREETYPE_LIBRARIES} glfw spdlog::spdlog)

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/include>"
        INTERFACE
        "$<INSTALL_INTERFACE:include>")

if(RB2D_INSTALL_CORE)
    include(cmake/InstallCore.cmake)
endif ()

