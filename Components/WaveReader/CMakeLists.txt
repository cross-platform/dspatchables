project(WaveReader)

add_definitions(-DSAMPLE_WAV_FILE="${CMAKE_CURRENT_SOURCE_DIR}/Sample.wav")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

file(GLOB srcs *.cpp)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Sample.wav DESTINATION lib/dspatch/components)
