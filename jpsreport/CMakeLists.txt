################################################################################
# Build jspreport executable
################################################################################
add_executable(jpsreport
    src/Analysis.cpp
    src/Analysis.h
    src/ArgumentParser.cpp
    src/ArgumentParser.h
    src/BuildingOld.cpp
    src/BuildingOld.h
    src/constants.h
    src/geometry_helper.cpp
    src/geometry_helper.h
    src/main.cpp
    src/methods/MeasurementArea.h
    src/methods/Method_A.cpp
    src/methods/Method_A.h
    src/methods/Method_B.cpp
    src/methods/Method_B.h
    src/methods/Method_C.cpp
    src/methods/Method_C.h
    src/methods/Method_D.cpp
    src/methods/Method_D.h
    src/methods/Method_I.cpp
    src/methods/Method_I.h
    src/methods/PedData.cpp
    src/methods/PedData.h
    src/methods/VoronoiDiagram.cpp
    src/methods/VoronoiDiagram.h
    $<$<PLATFORM_ID:Windows>:forms/jpscore.rc>
    $<$<PLATFORM_ID:Windows>:forms/JPScore.ico>
)

target_include_directories(jpsreport PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(jpsreport PRIVATE
    PYTHON="${PYTHON_EXECUTABLE}"
)

target_compile_options(jpsreport PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_link_libraries(jpsreport
    core
    tinyxml
    Boost::boost
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
    $<$<PLATFORM_ID:Windows>:wsock32>
)
