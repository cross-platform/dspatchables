project(InOut)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB srcs *.cpp)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
