file(GLOB_RECURSE SOURCES src/*.cpp src/*.hpp)

add_executable(FrontEnd_CLI ${SOURCES})

target_link_libraries(FrontEnd_CLI PRIVATE common)
