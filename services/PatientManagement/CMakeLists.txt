file(GLOB_RECURSE SOURCES src/*.cpp src/*.hpp)

add_executable(PatientManagement ${SOURCES})

target_link_libraries(PatientManagement PRIVATE common)
