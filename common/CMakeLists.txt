file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

set(GRPC_LOCAL_DIR ${PROJECT_SOURCE_DIR}/external/grpc/local)
list(INSERT CMAKE_PREFIX_PATH 0 ${GRPC_LOCAL_DIR}/lib/cmake)

find_package(Protobuf CONFIG REQUIRED PATHS ${GRPC_LOCAL_DIR}/lib/cmake/protobuf)
find_package(gRPC CONFIG REQUIRED PATHS ${GRPC_LOCAL_DIR}/lib/cmake/grpc)

add_library(common ${SOURCES})
target_sources(common PRIVATE ${SOURCES})

target_link_libraries(common
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
        Threads::Threads
)

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(GENERATED_DIR ${PROTO_DIR}/generated)

file(MAKE_DIRECTORY ${GENERATED_DIR})
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")
source_group("Proto Files" FILES ${PROTO_FILES})
target_sources(common PRIVATE ${PROTO_FILES})

find_program(PROTOC_EXECUTABLE
    protoc
    PATHS ${GRPC_LOCAL_DIR}/bin
    REQUIRED
)

find_program(GRPC_CPP_PLUGIN
    grpc_cpp_plugin
    PATHS ${GRPC_LOCAL_DIR}/bin
    REQUIRED
)

foreach(proto_file ${PROTO_FILES})
    get_filename_component(proto_name ${proto_file} NAME_WE)

    set(proto_src "${GENERATED_DIR}/${proto_name}.pb.cc")
    set(grpc_src "${GENERATED_DIR}/${proto_name}.grpc.pb.cc")
    set(proto_hdr "${GENERATED_DIR}/${proto_name}.pb.h")
    set(grpc_hdr "${GENERATED_DIR}/${proto_name}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${proto_src} ${grpc_src} ${proto_hdr} ${grpc_hdr}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS -I ${PROTO_DIR}
             --cpp_out=${GENERATED_DIR}
             --grpc_out=${GENERATED_DIR}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
             ${proto_file}
        DEPENDS ${proto_file}
        COMMENT "Generating protobuf and gRPC files for ${proto_name}"
    )

    source_group(
        TREE ${GENERATED_DIR}
        PREFIX "Generated Proto Files"
        FILES
            ${proto_src}
            ${grpc_src}
            ${proto_hdr}
            ${grpc_hdr}
)

    target_sources(common PRIVATE ${proto_src} ${grpc_src} ${proto_hdr} ${grpc_hdr})
endforeach()

# Include generated headers
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${GENERATED_DIR})

