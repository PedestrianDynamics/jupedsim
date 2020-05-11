################################################################################
# Build jspcore executable
################################################################################
add_executable(jpscore
    src/main.cpp
    forms/jpscore.rc
    forms/JPScore.ico
)
target_compile_options(jpscore PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_link_libraries(jpscore
    core
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

################################################################################
# Install targets
################################################################################
install(TARGETS jpscore
        DESTINATION bin
        COMPONENT applications)

set(CT_DATA_FILE_DIR "demos")
set(CT_FILES
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/01_corridor/corridor_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/01_corridor/corridor_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/02_bottleneck/bottleneck_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/02_bottleneck/bottleneck_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/03_corner/corner_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/03_corner/corner_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/03_corner/corner_routing.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/04_stairs/stairs_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/04_stairs/stairs_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/04_stairs/stairs_routing.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/05_events/events_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/05_events/events_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/05_events/events_list.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/05_events/events_routing.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/06_floorfield/floorfield_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/06_floorfield/floorfield_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/07_big_room/big_room_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/07_big_room/big_room_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/08_sources/source_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/08_sources/source_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/09_area/wa_triangle_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/09_area/wa_triangle_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/10_schedule/schedule_ini.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/10_schedule/schedule_geo.xml
        ${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/10_schedule/schedule.xml
        )

install(FILES ${CT_FILES}
  DESTINATION "jpscore_samples"
  COMPONENT jpscore_samples)

