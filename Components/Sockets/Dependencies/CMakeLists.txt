project(SocketsDependencies)

add_definitions(
    -DUSE_WEBSOCKET
)

file(GLOB srcs src/*.c)

add_library(
    ${PROJECT_NAME}
    cJSON.c
    mongoose.c
    ${srcs}
)
