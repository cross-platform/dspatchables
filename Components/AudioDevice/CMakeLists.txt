project(AudioDevice)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/RtAudio
)

file(GLOB srcs *.cpp RtAudio/*.cpp)
file(GLOB hdrs *.h RtAudio/*.h)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)

# Definition for RtAudio Windows, using direct sound
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-D__WINDOWS_WASAPI__)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Definition for RtAudio Linux, using ALSA
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-D__LINUX_ALSA__)

    find_library(ASOUND asound)
    if(NOT ASOUND)
        message(FATAL_ERROR "ALSA not found (Ensure that libasound2-dev is installed)")
    endif()

    target_link_libraries(
        ${PROJECT_NAME}
        ${ASOUND}
    )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Definition for RtAudio Mac OSX, using Core Audio
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-D__MACOSX_CORE__)

    target_link_libraries(
        ${PROJECT_NAME}
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
