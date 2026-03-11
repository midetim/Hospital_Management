file(GLOB_RECURSE SOURCES src/*.cpp src/*.hpp)

add_executable(RoomManagement ${SOURCES})

target_link_libraries(RoomManagement PRIVATE common)
