add_subdirectory(Core)
add_subdirectory(Ecs)
add_subdirectory(Graphics)
add_subdirectory(Util)

include(../cmake/External_GTest.cmake)
include_directories(${GTEST_INCLUDE_DIRS})
enable_testing()

set(TESTS_NAME run_all_tests)
set(SRC ${CORE_SRC} ${GRAPHICS_SRC} ${ECS_SRC} main.cpp)

add_executable(${TESTS_NAME} ${SRC})
target_link_libraries(${TESTS_NAME} gtest robot2D)

add_dependencies(${TESTS_NAME} gtest_ext)
add_test(NAME unit_tests COMMAND ${TESTS_NAME})