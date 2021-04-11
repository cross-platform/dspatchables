project(SocketOut)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies
)

file(GLOB srcs *.cpp)
file(GLOB hdrs *.h)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
    SocketsDependencies
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
