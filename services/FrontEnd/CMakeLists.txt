file(GLOB_RECURSE SOURCES src/*.cpp src/*.hpp)

add_executable(FrontEnd ${SOURCES})

target_link_libraries(FrontEnd PRIVATE common)
