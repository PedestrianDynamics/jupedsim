################################################################################
# jupedsim_obj
################################################################################
add_library(jupedsim_obj OBJECT
    include/jupedsim/jupedsim.h
    src/AgentIterator.hpp
    src/Conversion.cpp
    src/Conversion.hpp
    src/ErrorMessage.hpp
    src/agent.cpp
    src/build_info.cpp
    src/collision_free_speed_model.cpp
    src/error.cpp
    src/generalized_centrifugal_force_model.cpp
    src/geometry.cpp
    src/GeometryError.hpp
    src/journey.cpp
    src/logging.cpp
    src/operational_model.cpp
    src/simulation.cpp
    src/stage.cpp
    src/routing.cpp
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
# libjupedsim
################################################################################
add_library(jupedsim SHARED $<TARGET_OBJECTS:jupedsim_obj>)

target_include_directories(jupedsim
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include>
    PRIVATE src
)

target_link_libraries(jupedsim
    PRIVATE
        simulator
        common
)
set_property(TARGET jupedsim PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jupedsim PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

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

set(header_dest include/jupedsim)
install(
    FILES
        ${header_dest}/agent.h
        ${header_dest}/error.h
        ${header_dest}/geometry.h
        ${header_dest}/logging.h
        ${header_dest}/simulation.h
        ${header_dest}/types.h
        ${header_dest}/build_info.h
        ${header_dest}/export.h
        ${header_dest}/journey.h
        ${header_dest}/operational_model.h
        ${header_dest}/stage.h
        ${header_dest}/collision_free_speed_model.h
        ${header_dest}/generalized_centrifugal_force_model.h
        ${header_dest}/jupedsim.h
        ${header_dest}/routing.h
        ${header_dest}/transition.h
    DESTINATION ${header_dest}
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
