add_subdirectory(Core)
add_subdirectory(Ecs)
add_subdirectory(Graphics)
add_subdirectory(Util)

cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(TESTS_NAME robot2D-core-tests)
set(SRC ${CORE_SRC} ${ECS_SRC} ${GRAPHICS_SRC} ${UTIL_SRC} main.cpp)

add_executable(${TESTS_NAME} ${SRC})
target_link_libraries(${TESTS_NAME} PUBLIC GTest::gtest_main PRIVATE robot2D-core)

include(GoogleTest)
gtest_discover_tests(${TESTS_NAME})

# Separate processes keep once-per-process logging tests independent of test ordering.
add_executable(robot2D-logger-tests Util/LoggerTests.cpp)
target_link_libraries(robot2D-logger-tests PRIVATE robot2D-core)
foreach(logger_case IN ITEMS lazy repeat retry empty-path conflict concurrent)
    set(logger_directory "${CMAKE_CURRENT_BINARY_DIR}/scenarios/logger-${logger_case}")
    file(MAKE_DIRECTORY "${logger_directory}")
    add_test(NAME "core-logger-${logger_case}" COMMAND robot2D-logger-tests "${logger_case}")
    set_tests_properties("core-logger-${logger_case}" PROPERTIES
        WORKING_DIRECTORY "${logger_directory}" TIMEOUT 30 LABELS "core;util;headless")
endforeach()

add_executable(robot2D-assert-enabled-tests Util/AssertEnabledTests.cpp)
target_compile_definitions(robot2D-assert-enabled-tests PRIVATE RB_ASSERTS)
target_link_libraries(robot2D-assert-enabled-tests PRIVATE GTest::gtest_main robot2D-core)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/scenarios/assert-enabled")
gtest_discover_tests(robot2D-assert-enabled-tests
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/scenarios/assert-enabled")

add_executable(robot2D-assert-disabled-tests Util/AssertDisabledTests.cpp)
target_compile_definitions(robot2D-assert-disabled-tests PRIVATE NDEBUG)
target_link_libraries(robot2D-assert-disabled-tests PRIVATE GTest::gtest_main robot2D-core)
gtest_discover_tests(robot2D-assert-disabled-tests)

add_dependencies(robot2D-core-tests robot2D-logger-tests
    robot2D-assert-enabled-tests robot2D-assert-disabled-tests)

# Verify the normal Debug default, without forcing the legacy RB_ASSERTS opt-in.
add_executable(robot2D-assert-debug-default-tests Util/AssertEnabledTests.cpp)
if(MSVC)
    target_compile_options(robot2D-assert-debug-default-tests PRIVATE /UNDEBUG)
else()
    target_compile_options(robot2D-assert-debug-default-tests PRIVATE -UNDEBUG)
endif()
target_link_libraries(robot2D-assert-debug-default-tests PRIVATE GTest::gtest_main robot2D-core)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/scenarios/assert-debug-default")
gtest_discover_tests(robot2D-assert-debug-default-tests TEST_PREFIX "DebugDefault."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/scenarios/assert-debug-default")
add_dependencies(robot2D-core-tests robot2D-assert-debug-default-tests)
