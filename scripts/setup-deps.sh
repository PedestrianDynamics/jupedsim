#! /bin/bash
set -ex

boost_version="1.81.0"
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

setup_boost
