# One implementation shared by editor and standalone, without Editor/UI object files.
set(RB2D_RUNTIME_SOURCES
    AnimationParser.cpp AnimationPlayback.cpp AnimationSystem.cpp AnimatorSystem.cpp
    BoundingBox.cpp Components.cpp EditorCamera.cpp Errors.cpp FileApi.cpp Quad.cpp QuadTree.cpp
    RenderSystem.cpp Scene.cpp SceneEntity.cpp ScriptInteractor.cpp Task.cpp TaskFunction.cpp TaskQueue.cpp TextSystem.cpp
    UISystem.cpp Utils.cpp Uuid.cpp
    serializers/SceneSerializer.cpp serializers/EntitySerializer.cpp
    scripting/IScriptingEngineService.cpp scripting/MonoClassWrapper.cpp scripting/ScriptEngineData.cpp
    scripting/ScriptingEngine.cpp scripting/ScriptingGlue.cpp scripting/ScriptInstance.cpp)
file(GLOB runtime_physics "${CMAKE_CURRENT_SOURCE_DIR}/src/physics/*.cpp")
file(GLOB runtime_bootstrap "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/*.cpp")
list(TRANSFORM RB2D_RUNTIME_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
list(APPEND RB2D_RUNTIME_SOURCES ${runtime_physics} ${runtime_bootstrap})
add_library(robot2D-runtime STATIC ${RB2D_RUNTIME_SOURCES})
target_include_directories(robot2D-runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" ${MONO_INCLUDE_PATH})
target_link_libraries(robot2D-runtime PUBLIC robot2D-core yaml-cpp box2d tinyxml FileWatch ${MONO_LIBRARIES})
list(REMOVE_ITEM SRC ${RB2D_RUNTIME_SOURCES})
list(FILTER SRC EXCLUDE REGEX "/src/player/")
