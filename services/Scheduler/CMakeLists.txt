file(GLOB_RECURSE SOURCES src/*.cpp src/*.hpp)

add_executable(Scheduler ${SOURCES})

target_link_libraries(Scheduler PRIVATE common)
