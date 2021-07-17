project(VstHost)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/vstsdk
)

file(GLOB_RECURSE srcs *.cpp)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
