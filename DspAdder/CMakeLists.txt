project(DspAdder)

file(GLOB srcs *.cpp)
file(GLOB hdrs *.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)
