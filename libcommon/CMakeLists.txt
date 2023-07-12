add_library(common INTERFACE
    src/Unreachable.hpp
)

target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET common PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET common PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
