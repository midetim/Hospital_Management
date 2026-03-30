FROM --platform=linux/arm64 m4tt905/hospital-management-coe892:latest

WORKDIR /app

# Copy sources
COPY ./common ./common
COPY ./services/FrontEnd_CLI ./services/FrontEnd_CLI
COPY ./CMakeLists.txt ./

# Build ms
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && make -j2

CMD ["./build/FrontEnd_CLI"]