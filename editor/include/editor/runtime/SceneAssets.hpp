#pragma once
#include <editor/Scene.hpp>
namespace editor {
    void loadSceneAssets(const Scene::Ptr& scene);
    // CPU resources are loaded first; activation runs on the GL thread.
    void activateEntityAssets(const Scene::Ptr& scene, SceneEntity entity, bool strict = false);
}
