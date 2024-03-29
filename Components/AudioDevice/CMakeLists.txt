project(AudioDevice)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/RtAudio
)

file(GLOB srcs AudioDevice.cpp RtAudio/*.cpp)

add_library(
    ${PROJECT_NAME}
    ${srcs}
)

# Definition for RtAudio Windows, using WASAPI
if(WIN32)
    add_definitions(-D__WINDOWS_WASAPI__)

    target_link_libraries(
        ${PROJECT_NAME}
        ksuser
        mfplat
        mfuuid
        wmcodecdspuuid
    )
endif(WIN32)

# Definition for RtAudio Linux, using ALSA
if(UNIX AND NOT APPLE)
    add_definitions(-D__LINUX_ALSA__)

    find_library(ASOUND asound)
    if(NOT ASOUND)
        message(FATAL_ERROR "ALSA not found (Ensure that libasound2-dev is installed)")
    endif()

    target_link_libraries(
        ${PROJECT_NAME}
        ${ASOUND}
    )
endif(UNIX AND NOT APPLE)

# Definition for RtAudio Mac OSX, using Core Audio
if(APPLE)
    add_definitions(-D__MACOSX_CORE__)

    target_link_libraries(
        ${PROJECT_NAME}
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif(APPLE)

# AudioIn

add_library(
    AudioIn SHARED
    AudioIn.cpp
)

target_link_libraries(
    AudioIn
    ${PROJECT_NAME}
)

install(TARGETS AudioIn DESTINATION lib/dspatch/components)

# AudioOut

add_library(
    AudioOut SHARED
    AudioOut.cpp
)

target_link_libraries(
    AudioOut
    ${PROJECT_NAME}
)

install(TARGETS AudioOut DESTINATION lib/dspatch/components)
