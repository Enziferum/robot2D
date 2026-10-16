set(scriptcore_source "${CMAKE_CURRENT_SOURCE_DIR}/scripting/robot2D-ScriptCore/robot2D-ScriptCore")
file(GLOB_RECURSE scriptcore_sources CONFIGURE_DEPENDS "${scriptcore_source}/*.cs")
list(FILTER scriptcore_sources EXCLUDE REGEX "/(bin|obj)/")
add_mono_assembly(TARGET robot2D_ScriptCore
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/res/script"
    SOURCES ${scriptcore_sources}
    ADDITIONAL_ARGS -sdk:4.8 -langversion:7.2 -warn:4 -warnaserror+
        "$<IF:$<CONFIG:Debug>,-debug+,-optimize+>")
# Building the editor (or its test object library) must produce a matching API DLL.
add_dependencies(robot2D-editor-objects robot2D_ScriptCore)
add_dependencies(${PROJECT_NAME} robot2D_ScriptCore)
