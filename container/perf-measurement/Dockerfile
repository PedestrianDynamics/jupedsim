FROM ubuntu:23.04

# Install dependencies
RUN apt-get update && \
    apt-get install -y software-properties-common
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    wget \
    unzip \
    git \
    g++ \
    cmake \
    make \
    ninja-build \
    clang \
    libglm-dev \
    libopengl-dev \
    python3 \
    python3-pip \
    linux-tools-common \
    linux-tools-generic \
    flex \
    bison \
    binutils-dev \
    libunwind-dev \
    libslang2-dev \
    libdw-dev \
    linux-source

# Install perf from source
RUN cd /usr/src && tar -xf *.tar.bz2
RUN cd $(ls -d /usr/src/linux-source*/) && make -j -C tools/perf prefix=/usr/local install

# Install jpscore dependencies
COPY scripts/setup-deps.sh requirements.txt /opt/
RUN cd /opt && ./setup-deps.sh
RUN pip3 install -r /opt/requirements.txt --break-system-packages

# Clone FlameGraph repo
RUN cd /opt && git clone --depth=1 https://github.com/brendangregg/FlameGraph.git

RUN git config --global --add safe.directory '*'

# Copy perf run test
COPY container/perf-measurement/container_perf_test.py /opt/

ENTRYPOINT ["python3", "-u", "/opt/container_perf_test.py"]
CMD []
