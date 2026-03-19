cmake_minimum_required(VERSION 3.16)
project(Hospital_Management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Environment variables
set(SERVICE_DIR ${PROJECT_SOURCE_DIR}/services)
set(COMMON_DIR ${PROJECT_SOURCE_DIR}/common)
set(GRPC_INSTALL_DIR ${PROJECT_SOURCE_DIR}/external/grpc/local)

# Tell CMake to use project-local gRPC/Protobuf 
list(INSERT CMAKE_PREFIX_PATH 0 ${GRPC_INSTALL_DIR}/lib/cmake)

# Find installed packages
find_package(Protobuf CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/protobuf)
find_package(gRPC CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/grpc)
find_package(Threads REQUIRED)

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

