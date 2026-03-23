# ------------------------------------------------------------------
# Build configuration
# ------------------------------------------------------------------

# Build directory
BUILD_DIR := xcode-build

# CMake generator
CMAKE_GEN := Xcode

# Default target
all: build

# ------------------------------------------------------------------
# Build the project (Release by default)
# ------------------------------------------------------------------
build:
	cmake -S . -B $(BUILD_DIR) -G $(CMAKE_GEN) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) --config Release

# ------------------------------------------------------------------
# Refresh: clean + build
# ------------------------------------------------------------------
ref:
	rm -rf $(BUILD_DIR)
	$(MAKE) build

# ------------------------------------------------------------------
# Open Xcode project without building
# ------------------------------------------------------------------
open:
	cmake -S . -B $(BUILD_DIR) -G $(CMAKE_GEN)
	open $(BUILD_DIR)/Hospital_Management.xcodeproj

# ------------------------------------------------------------------
# Clean build directory
# ------------------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR)

# ------------------------------------------------------------------
# Rebuild protobuf and gRPC files (C++ & Python)
# ------------------------------------------------------------------
buf:
	rm -rf common/proto/generated/*
	external/grpc/local/bin/protoc -I=common/proto \
		--cpp_out=common/proto/generated \
		--grpc_out=common/proto/generated \
		--plugin=protoc-gen-grpc=external/grpc/local/bin/grpc_cpp_plugin \
		common/proto/*.proto
	external/grpc/local/bin/protoc -I=common/proto \
		--python_out=common/proto/generated \
		--grpc_python_out=common/proto/generated \
		--plugin=protoc-gen-grpc_python=external/grpc/local/bin/grpc_python_plugin \
		common/proto/*.proto

# ------------------------------------------------------------------
# Git helper
# ------------------------------------------------------------------
git:
ifndef MSG
	$(error MSG is undefined. Usage: make git MSG="commit message")
endif
	git add .
	git commit -m "$(MSG)"
	git push -u origin main

# ------------------------------------------------------------------
# Phony targets
# ------------------------------------------------------------------
.PHONY: all build ref open clean buf git
