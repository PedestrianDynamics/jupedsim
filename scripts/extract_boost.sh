#!/usr/bin/env bash
# Extract a minimal set of Boost modules from an official source tarball.
#
# Usage:
#   scripts/extract_boost.sh <tarball> <dest_dir> [module …]
#
# Arguments:
#   tarball   Path to boost_X_Y_Z.tar.gz downloaded from archives.boost.io
#   dest_dir  Directory to create; will contain include/boost/ and lib/cmake/
#   module …  Optional: override the list of modules/files to extract.
#             If omitted, the default list below is used.
#
# Example (from repo root):
#   scripts/extract_boost.sh /tmp/boost_1_88_0.tar.gz third-party/boost-1.88.0
#
# After running this script, update third-party/CMakeLists.txt to point at the
# new directory and remove the old one.

set -euo pipefail

TARBALL="${1:?Usage: $0 <tarball> <dest_dir> [module ...]}"
DEST="${2:?Usage: $0 <tarball> <dest_dir> [module ...]}"
shift 2

# Derive the in-tarball prefix from the filename (boost_1_88_0.tar.gz → boost_1_88_0)
BASENAME="$(basename "$TARBALL")"
PREFIX="${BASENAME%.tar.gz}"
# Also strip .tar.bz2 etc.
PREFIX="${PREFIX%.tar.bz2}"
PREFIX="${PREFIX%.tar.xz}"

# Derive the Boost version string (boost_1_88_0 → 1.88.0)
VERSION="$(echo "$PREFIX" | sed 's/boost_\([0-9]*\)_\([0-9]*\)_\([0-9]*\)/\1.\2.\3/')"

echo "Tarball prefix : $PREFIX"
echo "Boost version  : $VERSION"
echo "Destination    : $DEST"

# --- Default module list (directories + single-file headers) ----------------
# Derived by running scripts/scan_boost_deps.py for this project.
# Update after each CGAL or project source change that might add new Boost deps.

DEFAULT_DIRS=(
    algorithm
    any
    assert
    concept
    config
    container
    container_hash
    core
    describe
    detail
    exception
    functional
    fusion
    graph
    integer
    intrusive
    io
    iterator
    lexical_cast
    math
    move
    mp11
    mpl
    multiprecision
    numeric
    optional
    predef
    preprocessor
    property_map
    random
    range
    smart_ptr
    tuple
    type_index
    type_traits
    utility
)

DEFAULT_FILES=(
    aligned_storage.hpp
    any.hpp
    array.hpp
    assert.hpp
    call_traits.hpp
    concept_archetype.hpp
    concept_check.hpp
    config.hpp
    cstdfloat.hpp
    cstdint.hpp
    current_function.hpp
    foreach.hpp
    foreach_fwd.hpp
    integer.hpp
    integer_fwd.hpp
    integer_traits.hpp
    io_fwd.hpp
    lexical_cast.hpp
    limits.hpp
    next_prior.hpp
    noncopyable.hpp
    none.hpp
    none_t.hpp
    operators.hpp
    predef.h
    random.hpp
    ref.hpp
    static_assert.hpp
    throw_exception.hpp
    type.hpp
    type_index.hpp
    type_traits.hpp
    utility.hpp
    version.hpp
)
# ---------------------------------------------------------------------------

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

mkdir -p "$DEST/include/boost"

if [[ $# -gt 0 ]]; then
    # Caller supplied an explicit module list; split into dirs vs .hpp files.
    DIRS=()
    FILES=()
    for item in "$@"; do
        if [[ "$item" == *.hpp || "$item" == *.h ]]; then
            FILES+=("$item")
        else
            DIRS+=("$item")
        fi
    done
else
    DIRS=("${DEFAULT_DIRS[@]}")
    FILES=("${DEFAULT_FILES[@]}")
fi

echo "Extracting ${#DIRS[@]} directories and ${#FILES[@]} single-file headers…"

PATHS=()
for d in "${DIRS[@]}";  do PATHS+=("${PREFIX}/boost/${d}"); done
for f in "${FILES[@]}"; do PATHS+=("${PREFIX}/boost/${f}"); done

tar -xzf "$TARBALL" -C "$TMPDIR" "${PATHS[@]}"

cp -r "$TMPDIR/${PREFIX}/boost/." "$DEST/include/boost/"

# --- Generate CMake config files -------------------------------------------
echo "Writing CMake config files…"

mkdir -p \
    "$DEST/lib/cmake/Boost-${VERSION}" \
    "$DEST/lib/cmake/boost_headers-${VERSION}"

# boost_headers-config.cmake — provides Boost::headers (= Boost::boost)
cat > "$DEST/lib/cmake/boost_headers-${VERSION}/boost_headers-config.cmake" <<'EOF'
if(TARGET Boost::headers)
  return()
endif()

if(Boost_VERBOSE OR Boost_DEBUG)
  message(STATUS "Found boost_headers ${boost_headers_VERSION} at ${boost_headers_DIR}")
endif()

mark_as_advanced(boost_headers_DIR)

get_filename_component(_BOOST_CMAKEDIR "${CMAKE_CURRENT_LIST_DIR}/../" REALPATH)
get_filename_component(_BOOST_INCLUDEDIR "${_BOOST_CMAKEDIR}/../../include/" ABSOLUTE)

add_library(Boost::headers INTERFACE IMPORTED)
set_target_properties(Boost::headers PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_BOOST_INCLUDEDIR}"
)

unset(_BOOST_INCLUDEDIR)
unset(_BOOST_CMAKEDIR)
EOF

# boost_headers-config-version.cmake
cat > "$DEST/lib/cmake/boost_headers-${VERSION}/boost_headers-config-version.cmake" <<EOF
set(PACKAGE_VERSION ${VERSION})
if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
EOF

# BoostConfigVersion.cmake
cat > "$DEST/lib/cmake/Boost-${VERSION}/BoostConfigVersion.cmake" <<EOF
set(PACKAGE_VERSION ${VERSION})
if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
EOF

# BoostConfig.cmake — main entry point for find_package(Boost)
cat > "$DEST/lib/cmake/Boost-${VERSION}/BoostConfig.cmake" <<'BOOSTCONFIG'
# Minimal BoostConfig.cmake for a header-only Boost installation.
# Supports find_package(Boost REQUIRED) and the Boost::boost / Boost::headers targets.

if(Boost_VERBOSE OR Boost_DEBUG)
  message(STATUS "Found Boost ${Boost_VERSION} at ${Boost_DIR}")
endif()

macro(boost_find_component comp required quiet)
  set(_BOOST_QUIET)
  if(Boost_FIND_QUIETLY OR ${quiet})
    set(_BOOST_QUIET QUIET)
  endif()
  set(_BOOST_REQUIRED)
  if(${required} AND Boost_FIND_REQUIRED)
    set(_BOOST_REQUIRED REQUIRED)
  endif()
  set(__boost_comp_nv "${comp}")
  get_filename_component(_BOOST_CMAKEDIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
  find_package(boost_${__boost_comp_nv} ${Boost_VERSION} EXACT CONFIG
    ${_BOOST_REQUIRED} ${_BOOST_QUIET} HINTS ${_BOOST_CMAKEDIR})
  set(Boost_${comp}_FOUND ${boost_${__boost_comp_nv}_FOUND})
  string(TOUPPER ${comp} _BOOST_COMP_UPPER)
  set(Boost_${_BOOST_COMP_UPPER}_FOUND ${boost_${__boost_comp_nv}_FOUND})
  if(boost_${__boost_comp_nv}_FOUND)
    list(APPEND Boost_LIBRARIES Boost::${__boost_comp_nv})
  endif()
  unset(_BOOST_REQUIRED)
  unset(_BOOST_QUIET)
  unset(_BOOST_CMAKEDIR)
  unset(__boost_comp_nv)
  unset(_BOOST_COMP_UPPER)
endmacro()

boost_find_component(headers 1 0)

if(NOT boost_headers_FOUND)
  set(Boost_FOUND 0)
  set(Boost_NOT_FOUND_MESSAGE "Required dependency boost_headers not found.")
  return()
endif()

set(Boost_MAJOR_VERSION ${Boost_VERSION_MAJOR})
set(Boost_MINOR_VERSION ${Boost_VERSION_MINOR})
set(Boost_SUBMINOR_VERSION ${Boost_VERSION_PATCH})
set(Boost_VERSION_STRING ${Boost_VERSION})

get_target_property(Boost_INCLUDE_DIRS Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
set(Boost_LIBRARIES "")

cmake_policy(PUSH)
cmake_policy(SET CMP0057 NEW)

foreach(__boost_comp IN LISTS Boost_FIND_COMPONENTS)
  if(NOT "${__boost_comp}" STREQUAL "headers")
    boost_find_component(${__boost_comp} ${Boost_FIND_REQUIRED_${__boost_comp}} 0)
  endif()
endforeach()

if(NOT TARGET Boost::boost)
  add_library(Boost::boost INTERFACE IMPORTED)
  set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::headers)
  add_library(Boost::diagnostic_definitions INTERFACE IMPORTED)
  add_library(Boost::disable_autolinking INTERFACE IMPORTED)
  add_library(Boost::dynamic_linking INTERFACE IMPORTED)
  if(WIN32)
    set_property(TARGET Boost::diagnostic_definitions PROPERTY INTERFACE_COMPILE_DEFINITIONS "BOOST_LIB_DIAGNOSTIC")
    set_property(TARGET Boost::disable_autolinking PROPERTY INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB")
    set_property(TARGET Boost::dynamic_linking PROPERTY INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_DYN_LINK")
  endif()
endif()

cmake_policy(POP)
BOOSTCONFIG

echo "Done."
echo "  Include dir : $DEST/include/boost  ($(find "$DEST/include/boost" -maxdepth 1 | wc -l | tr -d ' ') entries)"
echo "  Size        : $(du -sh "$DEST/include/boost" | cut -f1)"
echo ""
echo "Next steps:"
echo "  1. Update third-party/CMakeLists.txt: change the Boost version and path"
echo "     to ${VERSION} / boost-${VERSION}"
echo "  2. Delete the old boost-X.Y.Z directory"
echo "  3. Re-run cmake to verify the new installation is found"
