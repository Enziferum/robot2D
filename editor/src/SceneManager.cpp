/*********************************************************************
(c) Alex Raag 2024
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <filesystem>
#include <robot2D/Util/Logger.hpp>

#include <editor/SceneManager.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/FileApi.hpp>

#include <editor/TaskQueue.hpp>
#include <editor/async/SceneLoadTask.hpp>

namespace editor {
    namespace {
        const std::string scenePath = "assets/scenes";
    }

    namespace fs = std::filesystem;

    SceneManager::SceneManager(robot2D::MessageBus& messageBus):
            m_messageBus{messageBus},
            m_associatedProject{nullptr},
            m_activeScene{nullptr},
            m_error{SceneManagerError::None}
            {}

    bool SceneManager::add(Project::Ptr&& project, IScriptInteractorFrom::WeakPtr scriptingEngine) {
        Scene::Ptr scene = std::make_shared<Scene>(m_messageBus);
        if(scene == nullptr) {
            m_error = SceneManagerError::MemoryAlloc;
            return false;
        }

        m_activeScene = scene;
        auto path = project->getPath();
        auto appendPath = combinePath(scenePath, project -> getStartScene());
        auto scenePath = combinePath(path, appendPath);
        m_activeScene -> setPath(scenePath);

        auto interactor = scriptingEngine.lock();
        if(!interactor)
            return false;

        std::string sceneName = "Unnamed Scene";
        SceneSerializer serializer(m_activeScene);
        if(!serializer.serialize(m_activeScene -> getPath(), sceneName, interactor)) {
            m_error = SceneManagerError::SceneSerialize;
            return false;
        }

        return true;
    }


    bool SceneManager::add(Project::Ptr&& project, const std::string& path, IScriptInteractorFrom::WeakPtr scriptingEngine) {
        Scene::Ptr scene = std::make_shared<Scene>(m_messageBus);
        if(scene == nullptr) {
            m_error = SceneManagerError::MemoryAlloc;
            return false;
        }
        SceneSerializer serializer(scene);
        std::string sceneName = "Unnamed Scene";
        auto interactor = scriptingEngine.lock();
        if(!interactor)
            return false;
        if(!serializer.serialize(path, sceneName, interactor)) {
            m_error = SceneManagerError::SceneSerialize;
            return false;
        }
        return true;
    }

    void SceneManager::loadSceneAsync(Project::Ptr project, std::string path, SceneLoadCallback&& callback,
                                      IScriptInteractorFrom::WeakPtr scriptInteractorFrom) {
        Scene::Ptr scene = std::make_shared<Scene>(m_messageBus);
        if(scene == nullptr) {
            m_error = SceneManagerError::MemoryAlloc;
            return;
        }

        scene -> setPath(path);
        scene -> setAssociatedProjectPath(project -> getPath());
        m_associatedProject = std::move(project);

        auto loadCallback = [this](const SceneLoadTask& task) {
            auto&& chainCallback = task.getChainCallback();
            auto scene = task.getScene();
            m_activeScene = scene;
            chainCallback(m_activeScene);
        };

        auto taskQueue = TaskQueue::GetQueue();
        taskQueue -> addAsyncTask<SceneLoadTask>(std::move(loadCallback), scene,
                                                 scriptInteractorFrom, std::move(callback));
    }

    bool SceneManager::remove() {
        if(!deleteDirectory(m_activeScene -> getPath())) {
            return false;
        }
        return true;
    }

    Scene::Ptr SceneManager::getActiveScene() const {
        return m_activeScene;
    }

    const SceneManagerError& SceneManager::getError() const {
        return m_error;
    }

    bool SceneManager::save(Scene::Ptr&& scene, IScriptInteractorFrom::WeakPtr scriptingEngine) {
        if(scene == nullptr)
            return false;

        SceneSerializer serializer{scene};
        std::string sceneName = "Unnamed Scene";
        auto interactor = scriptingEngine.lock();
        if(!interactor)
            return false;

        if(!serializer.serialize(scene-> getPath(), sceneName, interactor)) {
            m_error = SceneManagerError::SceneSerialize;
            return false;
        }

        return true;
    }

    Project::Ptr SceneManager::getAssociatedProject() const {
        return m_associatedProject;
    }

}
