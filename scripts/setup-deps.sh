#! /bin/bash
set -e
ROOT_DIR=$(pwd)
INSTALL_DIR=${ROOT_DIR}/deps

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
cmake --build . --target install -- -j$(nproc)
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
cmake --build . --target install -- -j$(nproc)
cd ../..
rm -rf v${SPDLOG_VER}.tar.gz spdlog-${SPDLOG_VER}

CATCH2_VER="2.9.2"
wget https://github.com/catchorg/Catch2/archive/v${CATCH2_VER}.tar.gz
tar xf v${CATCH2_VER}.tar.gz
cd Catch2-${CATCH2_VER}
mkdir build
cd build
cmake .. \
    -DBUILD_TESTING=OFF      \
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
cmake --build . --target install -- -j$(nproc)
cd ../..
rm -rf v${CATCH2_VER}.tar.gz Catch2-${CATCH2_VER}
