FROM hospital-base:latest

WORKDIR /app

# Copy sources
COPY ./common ./common
COPY ./services/Scheduler ./services/Scheduler
COPY ./CMakeLists.txt ./

# Build ms
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)

CMD ["./build/Scheduler"]