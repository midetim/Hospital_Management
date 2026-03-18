# Glob all Python files
file(GLOB PY_GUI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.py")

# Group them in Xcode
source_group("FrontEnd GUI" FILES ${PY_GUI_FILES})

# Create a dummy "library" target for Xcode visibility
add_library(FrontEnd_GUI_PythonFiles INTERFACE)
target_sources(FrontEnd_GUI_PythonFiles INTERFACE ${PY_GUI_FILES})