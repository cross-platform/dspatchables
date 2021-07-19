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

ExternalProject_Add(
    webrtc-audio-processing
    GIT_REPOSITORY https://github.com/cross-platform/webrtc-audio-processing.git
    CONFIGURE_COMMAND NINJA=${NINJA} ${MESON} -Dprefix=${CMAKE_CURRENT_BINARY_DIR} ../webrtc-audio-processing
    BUILD_COMMAND ${NINJA}
    INSTALL_COMMAND NINJA=${NINJA} ${MESON} install
    UPDATE_COMMAND ""
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    "${CMAKE_CURRENT_BINARY_DIR}/include/webrtc_audio_processing"
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
    ${CMAKE_CURRENT_BINARY_DIR}/lib/libwebrtc_audio_processing.a
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
