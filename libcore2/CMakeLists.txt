find_program(CARGO cargo REQUIRED)
message(STATUS "Found cargo at ${CARGO}")

file(GLOB_RECURSE rust_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.rs")
list(APPEND rust_sources 
    ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml
    ${CMAKE_CURRENT_SOURCE_DIR}/build.rs
    ${CMAKE_CURRENT_SOURCE_DIR}/cbindgen.toml
)
add_custom_command(
    COMMAND BINDINGS_OUT_PATH=${CMAKE_BINARY_DIR}/generated/rust ${CARGO} build
    DEPENDS ${rust_sources}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT ${CMAKE_BINARY_DIR}/generated/rust/libcore2.h
           ${CMAKE_CURRENT_SOURCE_DIR}/target/debug/libcore2.a
)
add_custom_target(core2_target ALL DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/target/debug/libcore2.a
    ${CMAKE_BINARY_DIR}/generated/rust/libcore2.h
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated/rust)
#file(TOUCH ${CMAKE_BINARY_DIR}/generated/rust/libcore2.h)

add_library(core2 INTERFACE)
target_link_libraries(core2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/target/debug/libcore2.a
)
add_dependencies(core2 core2_target)
target_include_directories(core2
    INTERFACE ${CMAKE_BINARY_DIR}/generated/rust
)
add_custom_target(clean-rust
    COMMAND ${CARGO} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
