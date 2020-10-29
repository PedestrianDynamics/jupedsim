set(methods
    methods/VoronoiDiagram.cpp
    methods/PedData.cpp
    methods/Method_A.cpp
    methods/Method_B.cpp
    methods/Method_C.cpp
    methods/Method_D.cpp
)

set(source_files
        Analysis.cpp
        general/ArgumentParser.cpp
)

set (header_files
    Analysis.h
    methods/MeasurementArea.h
    methods/VoronoiDiagram.h
    methods/PedData.h
    methods/Method_A.h
    methods/Method_B.h
    methods/Method_C.h
    methods/Method_D.h
    methods/ConfigData_D.h
    general/ArgumentParser.h
    general/Macros.h
)

add_library(report STATIC
    ${source_files}
    ${methods}
)

target_link_libraries(report PUBLIC
    shared
    $<$<BOOL:${USE_OPENMP}>:OpenMP::OpenMP_CXX>
    spdlog::spdlog
    fmt::fmt
    tinyxml
    fs
    Boost::boost
    git-info
)

target_compile_options(report PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_compile_definitions(report PUBLIC
    JPSREPORT_VERSION="${PROJECT_VERSION}"
)

set_property(TARGET report PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET report PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

add_executable(jpsreport
    main.cpp
    forms/jpsreport.rc
    forms/JPSreport.ico
)

target_compile_options(jpsreport PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_link_libraries(jpsreport
    report
)

set_property(TARGET jpsreport PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jpsreport PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
################################################################################
# Install targets
################################################################################
install(TARGETS jpsreport
        DESTINATION bin
        COMPONENT applications)

set(CT_DATA_FILE_DIR "demos")
file(GLOB CT_FILES "${CMAKE_SOURCE_DIR}/jpsreport/${CT_DATA_FILE_DIR}/*/*.xml")
install(FILES ${CT_FILES}
  DESTINATION "jpsreport_samples"
  COMPONENT jpsreport_samples)
