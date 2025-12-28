cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 23 REQUIRED)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Werror")
add_library(bytes INTERFACE Bits.h Common.h)

add_subdirectory(test)