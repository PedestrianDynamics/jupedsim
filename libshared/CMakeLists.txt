################################################################################
# Build lib
################################################################################

set(source_files
   Logger.cpp
)

set(header_files
   Logger.h
)

add_library(shared STATIC
        ${source_files}
        ${header_files}
)

target_link_libraries(shared PUBLIC
        spdlog::spdlog
        fmt::fmt
)

target_include_directories(shared
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(shared PRIVATE
        ${COMMON_COMPILE_OPTIONS}
)
