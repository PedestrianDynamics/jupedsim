################################################################################
# Build cnpy
################################################################################
if(JPSFIRE)
    find_package(ZLIB REQUIRED)
    add_library(cnpy STATIC
        cnpy/cnpy.cpp
    )
    target_link_libraries(cnpy PUBLIC ZLIB::ZLIB)
    # cnpy emits a few warnings during their build
    target_compile_options(cnpy PRIVATE
        # Currently (GCC9 vs Clang8) GCC is more specific and discerns
        # unused vs. set-but-unused
        $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-but-set-variable>
        -Wno-unused-variable
    )
    target_include_directories(cnpy PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/cnpy"
    )
endif()

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
target_include_directories(tinyxml PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyxml"
)

################################################################################
# Build visLibity
################################################################################
add_library(visilibity STATIC
    visiLibity/source_code/visilibity.cpp
    visiLibity/source_code/visilibity.hpp
)
target_include_directories(visilibity PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/visiLibity/source_code"
)

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
target_include_directories(poly2tri PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/poly2tri"
)
