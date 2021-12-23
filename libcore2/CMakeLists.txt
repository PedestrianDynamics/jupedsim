find_program(CARGO cargo REQUIRED)
message(STATUS "Found cargo at ${CARGO}")

file(GLOB_RECURSE rust_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.rs")

list(APPEND rust_sources 
    ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml
    ${CMAKE_CURRENT_SOURCE_DIR}/build.rs
    ${CMAKE_CURRENT_SOURCE_DIR}/cbindgen.toml
)

set(lib_out ${CMAKE_CURRENT_SOURCE_DIR}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}core2${CMAKE_STATIC_LIBRARY_SUFFIX})
set(header_out ${CMAKE_BINARY_DIR}/generated/rust/libcore2.h)

add_custom_command(
    COMMAND ${CMAKE_COMMAND} -E env BINDINGS_OUT_PATH=${CMAKE_BINARY_DIR}/generated/rust 
            ${CARGO} build --release
    DEPENDS ${rust_sources}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT ${header_out}
           ${lib_out}
)
add_custom_target(core2_target ALL DEPENDS 
    ${header_out}
    ${lib_out}
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated/rust)

add_library(core2 INTERFACE)
target_link_libraries(core2 INTERFACE
    ${lib_out}
)
add_dependencies(core2 core2_target)
target_include_directories(core2
    INTERFACE ${CMAKE_BINARY_DIR}/generated/rust
)
add_custom_target(clean-rust
    COMMAND ${CARGO} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
