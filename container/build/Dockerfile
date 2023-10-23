FROM quay.io/pypa/manylinux_2_28_x86_64:latest

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
        llvm-toolset \
        clang-tools-extra

COPY requirements.txt /opt/

RUN /opt/python/cp312-cp312/bin/pip install -r /opt/requirements.txt && \
    /opt/python/cp311-cp311/bin/pip install -r /opt/requirements.txt && \
    pipx install isort==5.12.0 --python python3.11 && \
    pipx install black==23.7.0 --python python3.11 

RUN git config --global --add safe.directory '*'
