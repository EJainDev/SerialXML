FROM gcc:16.2.0-trixie

ENV DEBIAN_FRONTEND=noninteractive

# LLVM apt repository for clangd-22
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    ca-certificates \
    wget \
    gnupg \
    python3 \
    python3-pip \
    python3-venv \
    ninja-build \
    libboost-serialization-dev \
    libcereal-dev \
    libgtest-dev \
    && \
    wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
    | gpg --dearmor -o /etc/apt/trusted.gpg.d/apt.llvm.org.gpg && \
    echo "deb http://apt.llvm.org/trixie/ llvm-toolchain-trixie-22 main" \
    > /etc/apt/sources.list.d/llvm.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    clangd-22 \
    && \
    rm -rf /var/lib/apt/lists/*

# Install the latest CMake from Kitware's official repository.
RUN wget https://github.com/Kitware/CMake/releases/download/v4.4.3/cmake-4.4.3-linux-x86_64.sh && bash cmake-4.4.3-linux-x86_64.sh --skip-license --prefix=/usr/local

# cmake-format / cmakelang
RUN python3 -m venv /opt/cmake-format && \
    /opt/cmake-format/bin/pip install --no-cache-dir --upgrade pip cmakelang && \
    ln -s /opt/cmake-format/bin/cmake-format /usr/local/bin/cmake-format && \
    ln -s /opt/cmake-format/bin/cmake-lint /usr/local/bin/cmake-lint

# Make clangd-22 the default clangd executable.
RUN ln -s /usr/bin/clangd-22 /usr/local/bin/clangd

WORKDIR /workspaces