add_library(common INTERFACE
    src/unreachable.hpp
    src/visitor.hpp
)

target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

