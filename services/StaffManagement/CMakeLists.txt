file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.hpp
)

add_executable(StaffManagement ${SOURCES})

target_link_libraries(StaffManagement
    PRIVATE
        common
        gRPC::grpc++
        protobuf::libprotobuf
)