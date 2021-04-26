project(SocketIn)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Mongoose
)

file(GLOB srcs *.cpp ../Mongoose/*.c)
file(GLOB hdrs *.h ../Mongoose/*.h)

add_library(
    ${PROJECT_NAME} SHARED
    ${srcs}
    ${hdrs}
)

target_link_libraries(
    ${PROJECT_NAME}
    DSPatch
)

if(WIN32)
    target_link_libraries(
        ${PROJECT_NAME}
        wsock32
    )
endif(WIN32)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/dspatch/components)
