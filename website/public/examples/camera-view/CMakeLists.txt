add_executable(robot2D-camera-view main.cpp)
target_link_libraries(robot2D-camera-view PRIVATE robot2D-core)
set_target_properties(robot2D-camera-view PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
