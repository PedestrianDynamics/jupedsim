#! /bin/bash
set -e
ROOT_DIR=$(pwd)
INSTALL_DIR=${ROOT_DIR}/deps

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    CPUS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CPUS=$(sysctl -n hw.logicalcpu)
fi

FMTLIB_VER="8.0.1"
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
cmake --build . --target install -- -j${CPUS}
cd ../..
rm -rf ${FMTLIB_VER}.tar.gz fmt-${FMTLIB_VER}

SPDLOG_VER="1.9.2"
wget https://github.com/gabime/spdlog/archive/v${SPDLOG_VER}.tar.gz
tar xf v${SPDLOG_VER}.tar.gz
cd spdlog-${SPDLOG_VER}
mkdir build
cd build
cmake .. \
    -DSPDLOG_BUILD_BENCH=OFF \
    -DSPDLOG_BUILD_TESTS=OFF \
    -DSPDLOG_FMT_EXTERNAL=ON \
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
cmake --build . --target install -- -j${CPUS}
cd ../..
rm -rf v${SPDLOG_VER}.tar.gz spdlog-${SPDLOG_VER}

CATCH2_VER="2.13.7"
wget https://github.com/catchorg/Catch2/archive/v${CATCH2_VER}.tar.gz
tar xf v${CATCH2_VER}.tar.gz
cd Catch2-${CATCH2_VER}
mkdir build
cd build
cmake .. \
    -DBUILD_TESTING=OFF      \
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
cmake --build . --target install -- -j${CPUS}
cd ../..
rm -rf v${CATCH2_VER}.tar.gz Catch2-${CATCH2_VER}

CLI11_VER="2.1.2"
wget https://github.com/CLIUtils/CLI11/archive/v${CLI11_VER}.tar.gz
tar xf v${CLI11_VER}.tar.gz
cd CLI11-${CLI11_VER}
mkdir build
cd build
cmake .. \
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
    -DCMAKE_BUILD_TYPE=Release \
    -DCLI11_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DCLI11_BUILD_EXAMPLES=OFF
cmake --build . --target install -- -j${CPUS}
cd ../..
rm -rf CLI11-${CLI11_VER} v${CLI11_VER}.tar.gz

GTEST_VER="1.11.0"
wget https://github.com/google/googletest/archive/refs/tags/release-${GTEST_VER}.tar.gz
tar xf release-${GTEST_VER}.tar.gz
cd googletest-release-${GTEST_VER}
mkdir build
cd build
cmake .. \
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
cmake --build . --target install -- -j${CPUS}
cd ../..
rm -rf release-${GTEST_VER}.tar.gz googletest-release-${GTEST_VER}
