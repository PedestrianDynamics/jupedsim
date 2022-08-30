#! /bin/bash
set -ex

googletest_version="1.11.0"
fmt_version="8.0.1"
boost_version="1.78.0"
# GLM has removed the cmake install target in 9.9.9.6 for unknown reasons
# on master the install target has been reintroduced. Instead of manually
# gatering the headers and static libs or maintaing a patch we will instead use
# a known good build from master. This is api compatible with 9.9.9.8 so we
# should(TM) have no issues with Windows.
glm_version="6ad79aae3eb5bf809c30bf1168171e9e55857e45"
poly2tri_version="3380f5c805dd25a06de21f7dacd4db529dbe07e7"
cgal_version="5.4"
pybind11_version="2.10.0"
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
        -DCMAKE_CXX_FLAGS="-fPIC -fvisibility=hidden" \
        -DFMT_DOC=OFF \
        -DFMT_TEST=OFF \
        -DCMAKE_INSTALL_PREFIX=${install_path} \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j ${CPUS}
    cmake --install .
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
        -DCMAKE_CXX_FLAGS="-fPIC -fvisibility=hidden" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_STATIC_LIBS=ON \
        -DGLM_TEST_ENABLE=OFF \
        -DCMAKE_INSTALL_PREFIX=${install_path}
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_poly2tri {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/jhasse/poly2tri/archive/${poly2tri_version}.zip
    unzip ${poly2tri_version}.zip
    cd poly2tri-${poly2tri_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_CXX_FLAGS="-fPIC" \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build . -- -j${CPUS}

    # Manual install since poly2tri cmake has no install target
    cp libpoly2tri.a ${install_path}/lib

    header_path=${install_path}/include/poly2tri
    mkdir -p ${header_path}/{common,sweep}
    cp ../poly2tri/poly2tri.h ${header_path}
    cp ../poly2tri/common/*.h ${header_path}/common
    cp ../poly2tri/sweep/*.h ${header_path}/sweep

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_cgal {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/CGAL/cgal/releases/download/v${cgal_version}/CGAL-${cgal_version}-library.tar.xz
    tar xf CGAL-${cgal_version}-library.tar.xz
    cd CGAL-${cgal_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_INSTALL_PREFIX=${install_path} \
        -DWITH_CGAL_Qt5=OFF \
        -DWITH_CGAL_ImageIO=OFF \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_pybind11 {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/pybind/pybind11/archive/refs/tags/v${pybind11_version}.tar.gz
    tar xf v${pybind11_version}.tar.gz
    cd pybind11-${pybind11_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_INSTALL_PREFIX=${install_path} \
        -DPYBIND11_TEST=OFF \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}
}

setup_boost
setup_googletest
setup_fmt
setup_glm
setup_poly2tri
setup_cgal
setup_pybind11
