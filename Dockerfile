# Stage 1: Build gRPC + Protobuf
FROM --platform=linux/arm64 ubuntu:22.04 AS grpc-builder

ARG GRPC_VERSION=v1.78.0

RUN apt-get update && apt-get install -y \
    build-essential cmake git autoconf libtool pkg-config curl unzip wget \
    python3 python3-pip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /grpc

RUN git clone --recurse-submodules -b $GRPC_VERSION https://github.com/grpc/grpc /grpc && \
    mkdir -p cmake/build && cd cmake/build && \
    cmake ../.. -DCMAKE_BUILD_TYPE=Release -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF && \
    make -j$(nproc) && make install

# Install Python gRPC tools so grpc_python_plugin is available
RUN pip3 install grpcio grpcio-tools && \
    cp /usr/local/lib/python3.*/site-packages/grpc_tools/_protoc_gen_grpc_python.py /usr/local/bin/grpc_python_plugin

# Stage 2: Base image for services
FROM --platform=linux/arm64 ubuntu:22.04 AS base

# Copy gRPC & Protobuf from builder stage
COPY --from=grpc-builder /usr/local /usr/local

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl-dev zlib1g \
    python3 python3-pip \
    && rm -rf /var/lib/apt/lists/*

ENV CPLUS_INCLUDE_PATH=/usr/local/include
ENV LIBRARY_PATH=/usr/local/lib
ENV PATH=/usr/local/bin:$PATH
