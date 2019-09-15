#! /bin/bash
set -e
ROOT_DIR=$(pwd)
INSTALL_DIR=${ROOT_DIR}/deps
EXTRA_ARGUMENTS_CMAKE_BUILD=""

if [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    echo "Found msys shell"
    EXTRA_ARGUMENTS_CMAKE_BUILD="--config Release"
fi

# Old cmake on Ubuntu 18.04 (Travis) does not yet understand 
# --parallel when building
export CMAKE_BUILD_PARALLEL_LEVEL=$(nproc)

FMTLIB_VER="6.0.0"
wget https://github.com/fmtlib/fmt/archive/${FMTLIB_VER}.tar.gz
tar xf ${FMTLIB_VER}.tar.gz
cd fmt-${FMTLIB_VER}
mkdir build
cd build
cmake .. \
    -DFMT_TEST=OFF \
    -DFMT_DOC=OFF \
    -DFMT_INSTALL=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
cmake --build . --target install ${EXTRA_ARGUMENTS_CMAKE_BUILD}
cd ../..
rm -rf ${FMTLIB_VER}.tar.gz fmt-${FMTLIB_VER}

SPDLOG_VER="1.3.1"
wget https://github.com/gabime/spdlog/archive/v${SPDLOG_VER}.tar.gz
tar xf v${SPDLOG_VER}.tar.gz
cd spdlog-${SPDLOG_VER}
mkdir build
cd build
cmake .. \
    -DSPDLOG_BUILD_EXAMPLES=OFF \
    -DSPDLOG_BUILD_BENCH=OFF \
    -DSPDLOG_BUILD_TESTS=OFF \
    -DSPDLOG_FMT_EXTERNAL=ON \
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
cmake --build . --target install ${EXTRA_ARGUMENTS_CMAKE_BUILD}
cd ../..
rm -rf v${SPDLOG_VER}.tar.gz spdlog-${SPDLOG_VER}

