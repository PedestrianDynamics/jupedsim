################################################################################
# Build lib
################################################################################

set(source_files
   Logger.cpp
)

set(header_files
   Logger.h
)

add_library(libshared STATIC
        ${source_files}
        ${header_files}
)

target_link_libraries(libshared PUBLIC
        spdlog::spdlog
        fmt::fmt
        git-info
)

target_compile_options(libshared PRIVATE
        ${COMMON_COMPILE_OPTIONS}
)

target_compile_definitions(libshared PUBLIC
        JPSCORE_VERSION="${PROJECT_VERSION}"
)