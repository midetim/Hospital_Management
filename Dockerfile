# Stage 1: Build gRPC + Protobuf
FROM ubuntu:22.04 AS grpc-builder

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

RUN pip3 install grpcio grpcio-tools

# Stage 2: Base
FROM ubuntu:22.04 AS base

COPY --from=grpc-builder /usr/local /usr/local

RUN apt-get update && apt-get install -y \
    build-essential cmake make git \
    libssl-dev zlib1g \
    python3 python3-pip \
    && rm -rf /var/lib/apt/lists/*

ENV CPLUS_INCLUDE_PATH=/usr/local/include
ENV LIBRARY_PATH=/usr/local/lib
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV PATH=/usr/local/bin:$PATH

WORKDIR /app

COPY . /app

# install GUI dependencies
RUN pip3 install -r /app/services/FrontEnd_GUI/requirements.txt

# Fix hardcoded gRPC tool paths expected by the project
RUN mkdir -p /app/external/grpc/local/bin && \
    ln -sf /usr/local/bin/grpc_python_plugin /app/external/grpc/local/bin/grpc_python_plugin && \
    ln -sf /usr/local/bin/grpc_cpp_plugin /app/external/grpc/local/bin/grpc_cpp_plugin && \
    ln -sf /usr/local/bin/protoc /app/external/grpc/local/bin/protoc

RUN make build

CMD ["/bin/bash"]