FROM ubuntu:22.04
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

ARG DEBIAN_FRONTEND=noninteractive

# Initialize args
ARG CMAKE_VER=3.23.2
ARG CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VER}/cmake-${CMAKE_VER}-linux-x86_64.tar.gz"
ARG HADOLINT_URL="https://github.com/hadolint/hadolint/releases/download/v2.10.0/hadolint-Linux-x86_64"

# Install base packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential=12.9ubuntu3 g++=4:11.2.0-1ubuntu1 llvm-15=1:15.0.7-0ubuntu0.22.04.3 \
    llvm-15-dev=1:15.0.7-0ubuntu0.22.04.3 python3=3.10.6-1~22.04 python-is-python3=3.9.2-2 python3-pip=22.0.2+dfsg-1ubuntu0.3 shellcheck=0.8.0-2 git=1:2.34.1-1ubuntu1.9 make=4.3-4.1build1 \
    clang-format=1:14.0-55~exp2 clang-tidy=1:14.0-55~exp2 wget=1.21.2-2ubuntu1 xz-utils=5.2.5-2ubuntu1 valgrind=1:3.18.1-1ubuntu2 clang-15=1:15.0.7-0ubuntu0.22.04.3 \
    lcov=1.15-1 curl=7.81.0-1ubuntu1.14 gnupg=2.2.27-3ubuntu2.1 flex=2.6.4-8build2 bison=2:3.8.2+dfsg-1build1 libstdc++-11-dev=11.4.0-1ubuntu1~22.04 \
    clang-tools=1:14.0-55~exp2 python2=2.7.18-3 rsync=3.2.7-0ubuntu0.22.04.2 graphviz=2.42.2-6 libc++-15-dev=1:15.0.7-0ubuntu0.22.04.3 gdb=12.0.90-0ubuntu1 \
    liblsan0=12.3.0-1ubuntu1~22.04 libtsan0=11.4.0-1ubuntu1~22.04 libasan5=9.5.0-1ubuntu1~22.04 libubsan1=12.3.0-1ubuntu1~22.04 && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Update clang alternative
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-15 80 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-15 80 && \
    update-alternatives --install /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-15 80 && \
    update-alternatives --install /usr/bin/llvm-objcopy llvm-objcopy /usr/bin/llvm-objcopy-15 80 && \
    update-alternatives --install /usr/bin/llvm-ranlib llvm-ranlib /usr/bin/llvm-ranlib-15 80 && \
    update-alternatives --install /usr/bin/llvm-size llvm-size /usr/bin/llvm-size-15 80 && \
    update-alternatives --install /usr/bin/llvm-strip llvm-strip /usr/bin/llvm-strip-15 80 && \
    update-alternatives --config clang && \
    update-alternatives --config clang++

WORKDIR /root

# Download run-clang-format wrapper
RUN git clone https://github.com/Sarcasm/run-clang-format.git
WORKDIR /root/run-clang-format
RUN git checkout 39081c9c42768ab5e8321127a7494ad1647c6a2f . && \
    ln -s /root/run-clang-format/run-clang-format.py /usr/bin/

WORKDIR /root

# Install custom CMake version
RUN curl -o "cmake_v${CMAKE_VER}" -L "${CMAKE_URL}" && \
    mkdir -p "/opt/cmake/${CMAKE_VER}" && \
    tar -xf "$(basename cmake_v${CMAKE_VER})" -C "/opt/cmake/${CMAKE_VER}" --strip-components=1 && \
    rm "$(basename cmake_v${CMAKE_VER})" && \
    ln -s /opt/cmake/"${CMAKE_VER}"/bin/* /usr/bin/ && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install hadolint - Dockerfile linter
RUN curl -o /usr/bin/hadolint -L "${HADOLINT_URL}" && \
    chmod 775 /usr/bin/hadolint

# Install base python packages
RUN pip3 --no-cache-dir install scipy==1.9.1 numpy==1.23.3 pylint==2.12.2

# Install doxygen
RUN curl https://www.doxygen.nl/files/doxygen-1.9.5.src.tar.gz -o doxygen-1.9.5.src.tar.gz && \
    tar -xvzf ./doxygen-1.9.5.src.tar.gz && \
    mkdir doxygen-1.9.5/build

WORKDIR /root/doxygen-1.9.5/build

RUN cmake -G "Unix Makefiles" .. && \
    make && \
    make install

WORKDIR /root
