file(GLOB_RECURSE SOURCES src/*.cpp src/*.hpp)

add_executable(ResourceManagement ${SOURCES})

target_link_libraries(ResourceManagement PRIVATE common)
