add_library(distance INTERFACE
    src/DistanceMap.hpp
)
target_include_directories(distance
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(distance INTERFACE
    fmt::fmt
)

set_property(TARGET distance PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET distance PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

if(BUILD_TESTS)
    add_executable(libdistance-tests
        test/TestDistanceMap.cpp
    )

    target_link_libraries(libdistance-tests PRIVATE
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        distance
    )

    target_compile_options(libdistance-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET libdistance-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libdistance-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()
