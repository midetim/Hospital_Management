cmake_minimum_required(VERSION 3.16)
project(Hospital_Management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffunction-sections -fdata-sections -flto -pthread")

# Linker flags
if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-dead_strip -pthread")
else()
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -pthread")
endif()

# Global folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Environment variables
set(SERVICE_DIR ${PROJECT_SOURCE_DIR}/services)
set(COMMON_DIR ${PROJECT_SOURCE_DIR}/common)
set(GRPC_INSTALL_DIR ${PROJECT_SOURCE_DIR}/external/grpc/local)

# Proto output directory
set(PROTO_BINARY_DIR ${CMAKE_BINARY_DIR}/generated_proto)

# Thread detection for AppleClang
if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_USE_PTHREADS_INIT ON)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
endif()

find_package(Threads REQUIRED)
set(ABSL_USE_SYSTEM_THREADS ON)

# Add local gRPC CMake paths first
list(INSERT CMAKE_PREFIX_PATH 0 ${GRPC_INSTALL_DIR}/lib/cmake)

# Finding protobuf and grpc package
find_package(Protobuf CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/protobuf)
message(STATUS "Found Protobuf version: ${Protobuf_VERSION}")

find_program(PROTOC_EXECUTABLE
    NAMES protoc
    PATHS ${GRPC_INSTALL_DIR}/bin
    NO_DEFAULT_PATH
    REQUIRED
)
message(STATUS "PROTOC executable: ${PROTOC_EXECUTABLE}")

find_package(gRPC CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/grpc)
message(STATUS "Found gRPC version: ${gRPC_VERSION}")

find_program(GRPC_CPP_PLUGIN
    NAMES grpc_cpp_plugin
    PATHS ${GRPC_INSTALL_DIR}/bin
    NO_DEFAULT_PATH
    REQUIRED
)
message(STATUS "gRPC C++ plugin: ${GRPC_CPP_PLUGIN}")

find_program(GRPC_PYTHON_PLUGIN
    NAMES grpc_python_plugin
    PATHS ${GRPC_INSTALL_DIR}/bin
    NO_DEFAULT_PATH
)
message(STATUS "gRPC Python plugin: ${GRPC_PYTHON_PLUGIN}")

set(PROTOC_EXECUTABLE ${PROTOC_EXECUTABLE} CACHE STRING "Path to local protoc")
set(GRPC_CPP_PLUGIN ${GRPC_CPP_PLUGIN} CACHE STRING "Path to local gRPC plugin")
set(GRPC_PYTHON_PLUGIN ${GRPC_PYTHON_PLUGIN} CACHE STRING "Path to local gRPC python plugin")

# Add subdirectories
add_subdirectory(${COMMON_DIR})
add_subdirectory(${SERVICE_DIR}/FrontEnd_CLI)
add_subdirectory(${SERVICE_DIR}/FrontEnd_GUI)
add_subdirectory(${SERVICE_DIR}/PatientManagement)
add_subdirectory(${SERVICE_DIR}/ResourceManagement)
add_subdirectory(${SERVICE_DIR}/RoomManagement)
add_subdirectory(${SERVICE_DIR}/StaffManagement)
add_subdirectory(${SERVICE_DIR}/Scheduler)
