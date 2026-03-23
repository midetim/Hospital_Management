# ------------------------------------------------------------------
# Proto Library (compiled once)
# ------------------------------------------------------------------

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated_proto)
set(PY_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated_python)

file(MAKE_DIRECTORY ${GENERATED_DIR})
file(MAKE_DIRECTORY ${PY_GENERATED_DIR})
file(WRITE ${PY_GENERATED_DIR}/__init__.py "")

file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

add_library(proto_library)

target_include_directories(proto_library
    PUBLIC
        ${GRPC_INSTALL_DIR}/include
        ${GENERATED_DIR}   # generated proto headers
)

target_link_libraries(proto_library
    PUBLIC
        gRPC::grpc++
        gRPC::grpc
        protobuf::libprotobuf
        protobuf::libprotoc
        protobuf::libprotobuf-lite
)

foreach(proto_file ${PROTO_FILES})
    get_filename_component(proto_name ${proto_file} NAME_WE)

    # --- C++ ---
    set(proto_src "${GENERATED_DIR}/${proto_name}.pb.cc")
    set(proto_hdr "${GENERATED_DIR}/${proto_name}.pb.h")
    set(grpc_src "${GENERATED_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdr "${GENERATED_DIR}/${proto_name}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${proto_src} ${proto_hdr} ${grpc_src} ${grpc_hdr}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS -I ${PROTO_DIR}
             --cpp_out=${GENERATED_DIR}
             --grpc_out=${GENERATED_DIR}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
             ${proto_file}
        DEPENDS ${proto_file}
        COMMENT "Generating C++ protobuf & gRPC files for ${proto_name}"
    )

    target_sources(proto_library PRIVATE ${proto_src} ${proto_hdr} ${grpc_src} ${grpc_hdr})
endforeach()

# ------------------------------------------------------------------
# Common Sources
# ------------------------------------------------------------------

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Client.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Schedule.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Service.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/grpc_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/time_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/Schedule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/time_utils.cpp
)

set(CPP_CLIENTS
    ${CMAKE_CURRENT_SOURCE_DIR}/clients/PatientManagementClient.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clients/ResourceManagementClient.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clients/RoomManagementClient.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clients/StaffManagementClient.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/clients/PatientManagementClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clients/ResourceManagementClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clients/RoomManagementClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clients/StaffManagementClient.cpp
)

add_library(common
    ${SOURCES}
    ${CPP_CLIENTS}
)

target_link_libraries(common
    PUBLIC
        proto_library
        Threads::Threads
)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/clients
        ${PROJECT_SOURCE_DIR}/external/json
)