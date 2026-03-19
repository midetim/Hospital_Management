# Glob all Python files
file(GLOB PY_GUI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.py")

# Group them in Xcode
#source_group("FrontEnd GUI" FILES ${PY_GUI_FILES})

# Create an OBJECT library (no linker step required)
add_library(FrontEnd_GUI OBJECT main.cpp ${PY_GUI_FILES})