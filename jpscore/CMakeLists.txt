################################################################################
# Build jspcore executable
################################################################################
add_executable(jpscore
    src/main.cpp
    forms/jpscore.rc
    forms/jpscore.ico
)
target_compile_options(jpscore PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_link_libraries(jpscore
    core
    shared
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
)
set_property(TARGET jpscore PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jpscore PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

if (WIN32)
    target_link_libraries(jpscore core wsock32)
endif (WIN32)


if(WIN32)
  if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT jpscore)
    message(STATUS "set start project for VS")
  endif()
endif()

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

    target_compile_definitions(jpscore_asan PRIVATE
        ${core_compile_definitions}
    )

    target_link_libraries(jpscore_asan
        ${jpscore_link_libraries}
        -fsanitize=address
    )
  set_property(TARGET jpscore_asan PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
  set_property(TARGET jpscore_asan PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()
