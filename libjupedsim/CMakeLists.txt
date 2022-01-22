################################################################################
# libjupedsim
################################################################################
add_library(jupedsim STATIC
    include/jupedsim/jupedsim.h
    src/jupedsim.cpp
)

target_compile_options(jupedsim PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_include_directories(jupedsim
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(jupedsim
    PUBLIC 
        simulator
) 
set_property(TARGET jupedsim PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jupedsim PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# libjupedsim unit tests
################################################################################
if (BUILD_TESTS)
    add_executable(libjupedsim-tests
        test/TestJupedsim.cpp
    )

    target_link_libraries(libjupedsim-tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        jupedsim
    )

    target_compile_options(libjupedsim-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    # Also allow the unit test access to the non-public header files of libjupedsim
    # This is required to construct some otherwise opqaue types in tests, e.g. ErrorMessage
    target_include_directories(libjupedsim-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    set_property(TARGET libjupedsim-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libjupedsim-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()
