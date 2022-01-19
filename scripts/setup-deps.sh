#! /bin/bash
set -ex

googletest_version="1.11.0"
fmt_version="8.0.1"
spdlog_version="1.9.2"
catch2_version="2.13.7"
cli11_version="2.1.2"
boost_version="1.78.0"
# GLM has removed the cmake install target in 9.9.9.6 for unknown reasons
# on master the install target has been reintroduced. Instead of manually
# gatering the headers and static libs or maintaing a patch we will instead use
# a known good build from master. This is api compatible with 9.9.9.8 so we
# should(TM) have no issues with Windows.
glm_version="6ad79aae3eb5bf809c30bf1168171e9e55857e45"
install_path=/usr/local

POSITIONAL=()
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --install-path)
      install_path="$2"
      shift # past argument
      shift # past value
      ;;
    *)    # unknown option
      POSITIONAL+=("$1") # save it in an array for later
      shift # past argument
      ;;
  esac
done

if [ ${install_path} ]; then
    if [ ! -d "${install_path}" ]; then
        mkdir -p ${install_path}
    fi
    install_path="$(cd ${install_path}; pwd)"
fi

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    CPUS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CPUS=$(sysctl -n hw.logicalcpu)
fi

function setup_boost {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}
    boost_version_string=${boost_version//[.]/_}
    wget https://boostorg.jfrog.io/artifactory/main/release/${boost_version}/source/boost_${boost_version_string}.tar.gz
    tar xf boost_${boost_version_string}.tar.gz
    cd boost_${boost_version_string}
    ./bootstrap.sh --prefix=${install_path} --with-libraries="headers"
    ./b2 --prefix=${install_path} install

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_googletest {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/google/googletest/archive/release-${googletest_version}.tar.gz
    tar xf release-${googletest_version}.tar.gz
    cd googletest-release-${googletest_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_INSTALL_PREFIX=${install_path} \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j ${CPUS}
    cmake --install .
    cd ${root}
    rm -rf ${temp_folder}
}

function setup_fmt {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/fmtlib/fmt/archive/${fmt_version}.tar.gz
    tar xf ${fmt_version}.tar.gz
    cd fmt-${fmt_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_CXX_FLAGS="-fPIC" \
        -DFMT_DOC=OFF \
        -DFMT_TEST=OFF \
        -DCMAKE_INSTALL_PREFIX=${install_path} \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j ${CPUS}
    cmake --install .
    cd ${root}
    rm -rf ${temp_folder}
}

function setup_spdlog {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/gabime/spdlog/archive/v${spdlog_version}.tar.gz
    tar xf v${spdlog_version}.tar.gz
    cd spdlog-${spdlog_version}
    mkdir build
    cd build
    cmake .. \
        -DSPDLOG_BUILD_BENCH=OFF \
        -DSPDLOG_BUILD_TESTS=OFF \
        -DSPDLOG_FMT_EXTERNAL=ON \
        -DCMAKE_PREFIX_PATH=${install_path} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${install_path}
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_catch2 {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/catchorg/Catch2/archive/v${catch2_version}.tar.gz
    tar xf v${catch2_version}.tar.gz
    cd Catch2-${catch2_version}
    mkdir build
    cd build
    cmake .. \
        -DBUILD_TESTING=OFF      \
        -DCMAKE_PREFIX_PATH=${install_path} \
        -DCMAKE_INSTALL_PREFIX=${install_path}
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_cli11 {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/CLIUtils/CLI11/archive/v${cli11_version}.tar.gz
    tar xf v${cli11_version}.tar.gz
    cd CLI11-${cli11_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_PREFIX_PATH=${install_path} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCLI11_BUILD_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX=${install_path} \
        -DCLI11_BUILD_EXAMPLES=OFF
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_glm {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/g-truc/glm/archive/${glm_version}.zip
    unzip ${glm_version}.zip
    cd glm-${glm_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_STATIC_LIBS=ON \
        -DGLM_TEST_ENABLE=OFF \
        -DCMAKE_INSTALL_PREFIX=${install_path}
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}

}

setup_boost
setup_googletest
setup_fmt
setup_spdlog
setup_catch2
setup_cli11
setup_glm
