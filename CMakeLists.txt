cmake_minimum_required(VERSION 2.8)

project(DSPatchables)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/link)

add_subdirectory(DspAdder)
add_subdirectory(DspAudioDevice)
add_subdirectory(DspGain)
add_subdirectory(DspOscillator)
add_subdirectory(DspWaveStreamer)
