project(Agc)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${WEBRTC_INCLUDE_DIR}
)

file(GLOB srcs *.cpp)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
)

add_dependencies(
    ${PROJECT_NAME}
    webrtc-audio-processing
)

target_link_libraries(
    ${PROJECT_NAME}
    ${WEBRTC_LIB}
)

if(WIN32)
    target_link_libraries(
        ${PROJECT_NAME}
        winmm
    )
endif(WIN32)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
