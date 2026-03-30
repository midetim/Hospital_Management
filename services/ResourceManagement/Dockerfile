FROM m4tt905/hospital-management-coe892:latest

WORKDIR /app

# Copy sources
COPY ./common ./common
COPY ./services/ResourceManagement ./services/ResourceManagement
COPY ./CMakeLists.txt ./

# Build ms
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && make -j2

CMD ["./build/ResourceManagement"]