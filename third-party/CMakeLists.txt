# Disable clang-tidy checks for all third party
set(CMAKE_CXX_CLANG_TIDY "")
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
