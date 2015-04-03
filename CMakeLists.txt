cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor")

project(DSPatchables)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/link)

add_subdirectory(DspOscillator)
