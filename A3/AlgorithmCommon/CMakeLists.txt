cmake_minimum_required(VERSION 3.22)

project(AlgorithmCommon)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_SHARED_LINKER_FLAGS  "-fPIC -shared -fvisibility=hidden -fvisibility-inlines-hidden")
add_library(AlgorithmCommon SHARED MyAlgorithm.cpp)
target_include_directories(AlgorithmCommon PUBLIC include)