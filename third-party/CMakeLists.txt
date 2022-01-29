################################################################################
# Build tinyxml
################################################################################
add_library(tinyxml STATIC
    tinyxml/tinystr.cpp
    tinyxml/tinyxml.cpp
    tinyxml/tinyxmlerror.cpp
    tinyxml/tinyxmlparser.cpp
    tinyxml/tinyxml.h
    tinyxml/tinystr.h
)
target_include_directories(tinyxml SYSTEM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyxml"
)
set_property(TARGET tinyxml PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET tinyxml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# Build visLibity
################################################################################
add_library(visilibity STATIC
    visiLibity/source_code/visilibity.cpp
    visiLibity/source_code/visilibity.hpp
)
target_include_directories(visilibity SYSTEM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/visiLibity/source_code"
)
set_property(TARGET visilibity PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET visilibity PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# Build poly2tri
################################################################################
add_library(poly2tri STATIC
    poly2tri/common/shapes.cpp
    poly2tri/common/shapes.h
    poly2tri/common/utils.h
    poly2tri/poly2tri.h
    poly2tri/sweep/advancing_front.cpp
    poly2tri/sweep/advancing_front.h
    poly2tri/sweep/cdt.cpp
    poly2tri/sweep/cdt.h
    poly2tri/sweep/sweep.cpp
    poly2tri/sweep/sweep.h
    poly2tri/sweep/sweep_context.cpp
    poly2tri/sweep/sweep_context.h
)
target_include_directories(poly2tri SYSTEM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/poly2tri"
)
set_property(TARGET poly2tri PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET poly2tri PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
