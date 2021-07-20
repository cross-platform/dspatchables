find_program(MESON meson)
if(NOT MESON)
    message(WARNING "Meson not found. Skipping WebRtc components...")
    return()
endif()

find_program(NINJA ninja)
if(NOT NINJA)
    message(WARNING "Ninja not found. Skipping WebRtc components...")
    return()
endif()

if(WIN32)
    add_definitions(-DWEBRTC_WIN -D_USE_MATH_DEFINES)
    set(CONFIGURE_COMMAND ${MESON} <INSTALL_DIR>/build --buildtype=release)
else()
    add_definitions(-DWEBRTC_POSIX)
    set(CONFIGURE_COMMAND NINJA=${NINJA} ${MESON} <INSTALL_DIR>/build --buildtype=release)
endif()

add_definitions(-DWEBRTC_AUDIO_PROCESSING_ONLY_BUILD)

set(WEBRTC_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/webrtc-audio-processing-prefix/src/webrtc-audio-processing)
set(WEBRTC_LIB ${CMAKE_CURRENT_BINARY_DIR}/webrtc-audio-processing-prefix/build/webrtc/modules/audio_processing/libwebrtc_audio_processing.a)

include(ExternalProject)

ExternalProject_Add(
    webrtc-audio-processing
    GIT_REPOSITORY https://github.com/cross-platform/webrtc-audio-processing.git
    GIT_SHALLOW 1
    CONFIGURE_COMMAND ${CONFIGURE_COMMAND}
    BUILD_COMMAND ${NINJA} -C <INSTALL_DIR>/build
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS ${WEBRTC_LIB}
    BUILD_IN_SOURCE 1
)

add_subdirectory(Aec)
add_subdirectory(Agc)
