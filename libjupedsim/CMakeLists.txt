################################################################################
# libjupedsim
################################################################################
add_library(jupedsim SHARED
    include/jupedsim/jupedsim.h
    src/jupedsim.cpp
)

target_compile_options(jupedsim PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_compile_definitions(jupedsim
    PRIVATE
        JUPEDSIM_API_EXPORTS
)
target_include_directories(jupedsim
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include>
    PRIVATE src
)

target_link_libraries(jupedsim
    PRIVATE
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
        simulator
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

################################################################################
# Installation
################################################################################
install(TARGETS jupedsim
    EXPORT jupedsimTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(
    FILES
        include/jupedsim/jupedsim.h
    DESTINATION include/jupedsim
)

install(EXPORT jupedsimTargets
    FILE jupedsimTargets.cmake
    NAMESPACE jupedsim::
    DESTINATION lib/cmake/jupedsim)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "jupedsimConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES "jupedsimConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/jupedsimConfigVersion.cmake"
    DESTINATION lib/cmake/jupedsim)