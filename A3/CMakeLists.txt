cmake_minimum_required(VERSION 3.22)
project(A3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_subdirectory(Algo_170154879_113332225_A)
add_subdirectory(Algo_170154879_113332225_B)
add_subdirectory(Simulation)

set(common_libs Algo_170154879_113332225_A Algo_170154879_113332225_B)
set(common_libs Algo_170154879_113332225_A)