FROM base

WORKDIR /app

# Copy sources
COPY . .

# Build ms
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)

CMD ["./build/Scheduler"]