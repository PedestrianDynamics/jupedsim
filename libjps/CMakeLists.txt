add_library(libjps SHARED
    src/jps.c
    )
set_target_properties(libjps PROPERTIES VERSION 0.0.1)
set_target_properties(libjps PROPERTIES PUBLIC_HEADER libjps/include/jps.h)

target_compile_options(libjps PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    )

target_link_libraries(libjps
    core
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
    )

target_include_directories(libjps PUBLIC
    ${core_include_directories}
    include
    )
