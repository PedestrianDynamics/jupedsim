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
