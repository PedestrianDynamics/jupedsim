add_library(jps SHARED
    src/jps.c
    )
set_target_properties(jps PROPERTIES VERSION 0.0.1)
set_target_properties(jps PROPERTIES PUBLIC_HEADER libjps/include/jps.h)

target_compile_options(jps PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    )

target_link_libraries(jps
    core
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
    )

target_include_directories(jps PUBLIC
    ${core_include_directories}
    include
    )
