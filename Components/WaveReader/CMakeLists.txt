project(WaveReader)

add_definitions(-DSAMPLE_WAV_FILE="${CMAKE_CURRENT_SOURCE_DIR}/Sample.wav")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB srcs *.cpp)
file(GLOB hdrs *.h)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)
