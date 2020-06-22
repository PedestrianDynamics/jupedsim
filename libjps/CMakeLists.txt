set(LIBJPS_HEADERS
    libjps/include/jps.h
    libjps/include/error.h
)

add_library(jps SHARED
    src/jps.c
    src/error.c
    )

set_target_properties(jps PROPERTIES VERSION 0.0.1)
set_target_properties(jps PROPERTIES PUBLIC_HEADER
    "${LIBJPS_HEADERS}"
    )

target_compile_options(jps PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    )

target_link_libraries(jps
    core
    )

target_include_directories(jps PUBLIC
    ${core_include_directories}
    include
    )
