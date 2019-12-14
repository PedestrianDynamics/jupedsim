set(methods
    methods/VoronoiDiagram.cpp
    methods/PedData.cpp
    methods/Method_A.cpp
    methods/Method_B.cpp
    methods/Method_C.cpp
    methods/Method_D.cpp
    methods/Method_I.cpp
    methods/Method_J.cpp
)

set(source_files
    Analysis.cpp
    IO/OutputHandler.cpp
    general/ArgumentParser.cpp
    geometry/Building.cpp
    geometry/Line.cpp
    geometry/Point.cpp
    geometry/Transition.cpp
    geometry/Crossing.cpp
    geometry/NavLine.cpp
    geometry/Room.cpp
    geometry/Wall.cpp
    geometry/Hline.cpp
    geometry/Obstacle.cpp
    geometry/SubRoom.cpp
    geometry/Goal.cpp
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
    methods/Method_I.h
    methods/Method_J.h
    IO/OutputHandler.h
    general/ArgumentParser.h
    general/Macros.h
    geometry/Building.h
    geometry/Line.h
    geometry/Point.h
    geometry/Transition.h
    geometry/Crossing.h
    geometry/NavLine.h
    geometry/Room.h
    geometry/Wall.h
    geometry/Hline.h
    geometry/Obstacle.h
    geometry/SubRoom.h
    geometry/Goal.h
)

add_library(report STATIC
    ${source_files}
    ${methods}
)

target_link_libraries(report PUBLIC
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

add_executable(jpsreport
    main.cpp
)

target_compile_options(jpsreport PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_link_libraries(jpsreport
    report
)
