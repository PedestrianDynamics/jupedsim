#!/usr/bin/env bash
set -e
echo "Sanitizer build using ${CC} and ${CXX}"
numcpus=$(nproc)

# Resolve the shared ASAN runtime for LD_PRELOAD.
# Required because Python is not built with sanitizers —
# the runtime must be loaded before Python starts.
ASAN_LIB=$(${CC} -print-file-name=libasan.so)
if [[ "${CC}" == clang* ]]; then
    ASAN_RT_DIR=$(${CC} -print-runtime-dir)
    ASAN_LIB=$(ls ${ASAN_RT_DIR}/libclang_rt.asan-*.so 2>/dev/null | head -1)
fi

mkdir build && cd build
cmake .. -DBUILD_TESTS=ON -DBUILD_WITH_SANITIZERS=ON
cmake --build . -- -j ${numcpus} -- VERBOSE=1

# C++ unit tests — run directly under sanitizers
cmake --build . -t unittests -- -j ${numcpus} -- VERBOSE=1

# Python system tests — need LD_PRELOAD so the ASAN runtime
# is loaded before the Python interpreter.
# PYTHONMALLOC=malloc bypasses pymalloc so ASAN can track
# all allocations. detect_leaks=0 suppresses false-positive
# leak reports from CPython internals.
export LD_PRELOAD=${ASAN_LIB}
export PYTHONMALLOC=malloc
export ASAN_OPTIONS="${ASAN_OPTIONS}:detect_leaks=0"
cmake --build . -t systemtests -- -j ${numcpus} -- VERBOSE=1
