cmake_minimum_required(VERSION 3.16)
project(Hospital_Management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build if not specified
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

# Environment variables
set(SERVICE_DIR ${PROJECT_SOURCE_DIR}/services)
set(COMMON_DIR ${PROJECT_SOURCE_DIR}/common)
set(GRPC_INSTALL_DIR ${PROJECT_SOURCE_DIR}/external/grpc/local)

# Force CMake to detect threads correctly on AppleClang/Xcode
if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_USE_PTHREADS_INIT ON)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
endif()

# Ensure Abseil uses Threads
find_package(Threads REQUIRED)
set(ABSL_USE_SYSTEM_THREADS ON)

# Tell CMake to use project-local gRPC/Protobuf
list(INSERT CMAKE_PREFIX_PATH 0 ${GRPC_INSTALL_DIR}/lib/cmake)

# Find installed packages
find_package(Protobuf CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/protobuf)
find_package(gRPC CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/grpc)


# Common files
add_subdirectory(${COMMON_DIR})

# Adding each microservice as its own subdir
add_subdirectory(${SERVICE_DIR}/FrontEnd_CLI)
add_subdirectory(${SERVICE_DIR}/FrontEnd_GUI)
add_subdirectory(${SERVICE_DIR}/PatientManagement)
add_subdirectory(${SERVICE_DIR}/ResourceManagement)
add_subdirectory(${SERVICE_DIR}/RoomManagement)
add_subdirectory(${SERVICE_DIR}/StaffManagement)
add_subdirectory(${SERVICE_DIR}/Scheduler)

