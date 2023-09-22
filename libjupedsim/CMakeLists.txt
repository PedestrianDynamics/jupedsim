################################################################################
# jupedsim_obj
################################################################################
add_library(jupedsim_obj OBJECT
    include/jupedsim/jupedsim.h
    src/AgentIterator.hpp
    src/Conversion.cpp
    src/Conversion.hpp
    src/ErrorMessage.hpp
    src/jupedsim.cpp
)

target_compile_options(jupedsim_obj PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_compile_definitions(jupedsim_obj
    PRIVATE
        JUPEDSIM_API_EXPORTS
)
target_include_directories(jupedsim_obj
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include>
    PRIVATE src
)

target_link_libraries(jupedsim_obj
    PRIVATE
        simulator
        common
)
set_property(TARGET jupedsim_obj PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jupedsim_obj PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

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
        jupedsim_obj
        simulator
    )

    target_compile_options(libjupedsim-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    # Also allow the unit test access to the non-public header files of jupedsim_obj
    # This is required to construct some otherwise opqaue types in tests, e.g. ErrorMessage
    target_include_directories(libjupedsim-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    set_property(TARGET libjupedsim-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libjupedsim-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()

################################################################################
# libjupedsim shared
###############################################################################
add_library(jupedsim_shared SHARED $<TARGET_OBJECTS:jupedsim_obj>)

target_include_directories(jupedsim_shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include>
    PRIVATE src
)
set_target_properties(jupedsim_shared PROPERTIES OUTPUT_NAME jupedsim)

target_link_libraries(jupedsim_shared
    PRIVATE
        simulator
        common
)
set_property(TARGET jupedsim_shared PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jupedsim_shared PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# libjupedsim static
################################################################################
add_library(jupedsim_static STATIC 
    $<TARGET_OBJECTS:jupedsim_obj>
    $<TARGET_OBJECTS:simulator>
)
set_target_properties(jupedsim_static PROPERTIES OUTPUT_NAME jupedsim)

target_include_directories(jupedsim_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include>
    PRIVATE src
)

target_include_directories(jupedsim_static
    PRIVATE
        common
)
set_property(TARGET jupedsim_static PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jupedsim_static PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# Installation
################################################################################
install(TARGETS jupedsim_shared
    EXPORT jupedsimTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(TARGETS jupedsim_static
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
