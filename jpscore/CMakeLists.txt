################################################################################
# Build jpscore executable
################################################################################
add_executable(jpscore
    src/main.cpp
    $<$<PLATFORM_ID:Windows>:forms/jpscore.rc>
    $<$<PLATFORM_ID:Windows>:forms/JPScore.ico>
)

target_compile_options(jpscore PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_link_libraries(jpscore
    core
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
    $<$<PLATFORM_ID:Windows>:wsock32>
)

################################################################################
# Build jspcore_asan executable - jpscore with address sanitizer
################################################################################
if(BUILD_WITH_ASAN)
    get_target_property(jpscore_source jpscore SOURCES)
    get_target_property(jpscore_compile_options jpscore COMPILE_OPTIONS)
    get_target_property(jpscore_link_libraries jpscore LINK_LIBRARIES)
    list(REMOVE_ITEM jpscore_link_libraries core)
    list(APPEND jpscore_link_libraries core_asan)
    add_executable(jpscore_asan
        ${jpscore_source}
    )

    target_compile_options(jpscore_asan PRIVATE
        ${jpscore_compile_options}
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fsanitize=address
    )

    target_link_libraries(jpscore_asan
        ${jpscore_link_libraries}
        -fsanitize=address
    )
endif()
