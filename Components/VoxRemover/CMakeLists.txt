project(VoxRemover)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
)

file(GLOB srcs *.cpp kissfft/*.cpp kissfft/*.c)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
