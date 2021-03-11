cmake_minimum_required(VERSION 3.17)
project(ICT1007_Lab4 C)

set(CMAKE_C_STANDARD 11)
set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

include_directories(.)

add_executable(ICT1007_Lab4 producer_consumer.c)