cmake_minimum_required(VERSION 3.16)

project(F1_ERS_Optimizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

add_library(ERSCore
    src/track-model/segment.cpp
    src/track-model/track.cpp
    src/track-model/fast-corner.cpp
    src/track-model/slow-corner.cpp
    src/track-model/straight.cpp
    src/battery.cpp
    src/car.cpp
    src/optimizer.cpp
    src/physics.cpp
)
target_include_directories(ERSCore PUBLIC include)

add_executable(ERSOptimizer src/main.cpp)
target_link_libraries(ERSOptimizer PRIVATE ERSCore)

set_target_properties(ERSOptimizer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin" )

# ---- GoogleTest ----
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests
    test/test_battery.cpp
    test/test_car.cpp
    test/test_physics.cpp
)
target_link_libraries(unit_tests PRIVATE ERSCore GTest::gtest_main)

set_target_properties(unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin" )

include(GoogleTest)
gtest_discover_tests(unit_tests)