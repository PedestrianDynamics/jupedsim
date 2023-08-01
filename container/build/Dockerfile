FROM quay.io/pypa/manylinux_2_28_x86_64:2023-07-30-0f4b381

RUN dnf upgrade && \
    dnf install -y \
        wget \
        unzip \
        git \
        gcc \
        cmake \
        make \
        ninja-build \
        llvm \
        clang-tools-extra

RUN /opt/python/cp310-cp310/bin/pip install pytest && \
    /opt/python/cp311-cp311/bin/pip install pytest

RUN git config --global --add safe.directory '*'
