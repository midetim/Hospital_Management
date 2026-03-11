# Build directory
BUILD_DIR := xcode-build

# CMake generator
CMAKE_GEN := Xcode

# Default Target
all: ref

# Opens the project
xcode:
	cmake -S . -B $(BUILD_DIR) -G $(CMAKE_GEN)
	cmake --build $(BUILD_DIR)
	
	
# Refresh the project: delete build dir, regenerate, and build
ref:
	rm -rf $(BUILD_DIR)
	$(MAKE) xcode

# Open Xcode project without building
open: xcode
	open $(BUILD_DIR)/Hospital_Management.xcodeproj

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

# Remake protobuf files
buf:
	rm -rf common/proto/generated/*.pb.*
	external/grpc/local/bin/protoc -I=common/proto --cpp_out=common/proto/generated --grpc_out=common/proto/generated --plugin=protoc-gen-grpc=external/grpc/local/bin/grpc_cpp_plugin common/proto/*.proto

.PHONY: all xcode ref open clean
