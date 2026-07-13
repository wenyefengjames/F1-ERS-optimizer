cmake_minimum_required(VERSION 3.16)

project(F1_ERS_Optimizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ERSOptimizer
    src/main.cpp
    src/track-model/segment.cpp
    src/track-model/track.cpp
    src/track-model/fast-corner.cpp
    src/track-model/slow-corner.cpp
    src/track-model/straight.cpp
    src/battery.cpp
    src/car.cpp
)

set_target_properties(ERSOptimizer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin" )