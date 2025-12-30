project(bytes_python VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PYTHON_FIND_VIRTUALENV ONLY)
find_package(Python 3.13 REQUIRED COMPONENTS Development Interpreter)
if(Python_NOT_FOUND)
    message(FATAL_ERROR "Unable to find python from a virtual environment. Has a virtual environment been defined and activated")
endif()
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(pybind11)
message(STATUS "Pybind configured with ${Python3_EXECUTABLE}")

include_directories(
    ${Python_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpp
)

file(GLOB bindingSrcs "./bitview/*.cpp")
pybind11_add_module(bitview MODULE ${bindingSrcs})
set_target_properties(bitview PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bitview)