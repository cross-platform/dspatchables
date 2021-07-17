project(VoxRemover)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
)

file(GLOB_RECURSE srcs *.cpp *.c)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
