add_executable(robot2D-sprite-animation main.cpp)
target_link_libraries(robot2D-sprite-animation PRIVATE robot2D-core)
set_target_properties(robot2D-sprite-animation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
