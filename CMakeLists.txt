cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -pedantic -Wall -Wextra -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -Wno-gnu-zero-variadic-macro-arguments -Wno-vla-extension")

project(DSPatchables)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/DSPatch/include)

add_subdirectory(DSPatch)
add_subdirectory(DSPatchables)
