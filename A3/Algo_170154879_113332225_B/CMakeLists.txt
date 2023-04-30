cmake_minimum_required(VERSION 3.22)

project(Algo_170154879_113332225_B)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_SHARED_LINKER_FLAGS "-fPIC -shared -fvisibility=hidden -fvisibility-inlines-hidden")
add_library(Algo_170154879_113332225_B SHARED Algo_170154879_113332225_B.cpp)
target_include_directories(Algo_170154879_113332225_B PUBLIC include)