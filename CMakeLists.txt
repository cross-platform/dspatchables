cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(DSPatchables)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/DSPatch/include)

add_subdirectory(DSPatch)
add_subdirectory(DSPatchables)
