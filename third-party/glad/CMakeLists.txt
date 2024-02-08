add_library(glad STATIC
    glad/glad.h
    glad/glad.c
    glad/khrplatform.h
)

target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})