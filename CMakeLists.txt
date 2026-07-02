cmake_minimum_required(VERSION 3.16)

project(F1_ERS_Optimizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ERSOptimizer src/main.cpp)

set_target_properties(ERSOptimizer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin" )