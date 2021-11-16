################################################################################
# Build cnpy
################################################################################
add_library(cnpy STATIC
    cnpy/cnpy.cpp
)
target_link_libraries(cnpy PUBLIC ZLIB::ZLIB)
# cnpy emits a few warnings during their build
target_compile_options(cnpy PRIVATE
    # Currently (GCC9 vs Clang8) GCC is more specific and discerns
    # unused vs. set-but-unused
    $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-but-set-variable>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wno-unused-variable>
)
target_include_directories(cnpy PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/cnpy"
)
set_property(TARGET cnpy PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET cnpy PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

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
set_property(TARGET tinyxml PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET tinyxml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

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
target_include_directories(poly2tri PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/poly2tri"
)
set_property(TARGET poly2tri PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET poly2tri PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# Build sqlite
################################################################################
add_library(sqlite STATIC
    sqlite-amalgamation-3360000/sqlite3.c
)
target_include_directories(sqlite PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-amalgamation-3360000
)
target_compile_definitions(sqlite PUBLIC
    SQLITE_DQS=0
    SQLITE_THREADSAFE=0
    SQLITE_DEFAULT_MEMSTATUS=0
    SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
    SQLITE_LIKE_DOESNT_MATCH_BLOBS
    SQLITE_MAX_EXPR_DEPTH=0
    SQLITE_OMIT_DECLTYPE
    SQLITE_OMIT_DEPRECATED
    SQLITE_OMIT_PROGRESS_CALLBACK
    SQLITE_OMIT_SHARED_CACHE
    SQLITE_USE_ALLOCA
    SQLITE_OMIT_AUTOINIT
)
set_property(TARGET sqlite PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET sqlite PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

add_executable(sqlite3
    sqlite-amalgamation-3360000/shell.c
)
target_link_libraries(sqlite3
    sqlite
)
set_property(TARGET sqlite3 PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET sqlite3 PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
