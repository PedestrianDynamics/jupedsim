################################################################################
# Build lib
################################################################################
add_library(shared STATIC
    src/IteratorPair.hpp
    src/Logger.cpp
    src/Logger.hpp
    src/ResultHandling.cpp
    src/ResultHandling.hpp
    src/UniqueID.hpp
    src/Macros.hpp
    src/Enum.hpp
    src/HashCombine.hpp
)

target_link_libraries(shared PUBLIC
    fmt::fmt
    tinyxml
)

target_include_directories(shared
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(shared PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

set_property(TARGET shared PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET shared PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# unit tests
################################################################################
if(BUILD_TESTS)
    add_executable(libshared-tests
        test/ResultHandlingTest.cpp
        test/TestUniqueID.cpp
    )
    target_link_libraries(libshared-tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        shared
    )

    target_compile_options(libshared-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET libshared-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libshared-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()
