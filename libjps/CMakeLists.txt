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

################################################################################
# libjps unit tests
################################################################################
if (BUILD_CPPUNIT_TEST)
    find_package(GTest REQUIRED)

    add_executable(libjps_unittests
            test/gtest/jps_api.cpp
            )

    target_link_libraries(libjps_unittests PRIVATE
        jps
        GTest::GTest
        GTest::Main
    )

    target_compile_options(unittests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET unittests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET unittests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()
