project(Agc)

include(ExternalProject)

if(WIN32)
    add_definitions(-DWEBRTC_WIN -DNOMINMAX)
else()
    add_definitions(-DWEBRTC_POSIX)
endif()
add_definitions(-DWEBRTC_AUDIO_PROCESSING_ONLY_BUILD)

find_program(MESON meson)
find_program(NINJA ninja)

set(WEBRTC_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/webrtc-audio-processing-prefix/src/webrtc-audio-processing)
set(WEBRTC_LIB ${CMAKE_CURRENT_BINARY_DIR}/webrtc-audio-processing-prefix/src/webrtc-audio-processing-build/webrtc/modules/audio_processing/libwebrtc_audio_processing.a)

ExternalProject_Add(
    webrtc-audio-processing
    GIT_REPOSITORY https://github.com/cross-platform/webrtc-audio-processing.git
    CONFIGURE_COMMAND NINJA=${NINJA} ${MESON} -Dprefix=${CMAKE_CURRENT_BINARY_DIR} ../webrtc-audio-processing
    BUILD_COMMAND ${NINJA}
    BUILD_BYPRODUCTS ${WEBRTC_LIB}
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
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
    DSPatch
    ${WEBRTC_LIB}
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
