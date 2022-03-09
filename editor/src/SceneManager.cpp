/*********************************************************************
(c) Alex Raag 2021
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

#include <editor/SceneManager.hpp>
#include <editor/FileApi.hpp>
#include <editor/SceneSerializer.hpp>

namespace editor {
    namespace {
        const std::string scenePath = "assets/scenes";
    }

    SceneManager::SceneManager(robot2D::MessageBus& messageBus):
            m_messageBus{messageBus},
            m_activeScene{nullptr},
            m_associatedProject{nullptr},
            m_error{SceneManagerError::None} {

    }

    bool SceneManager::add(Project::Ptr&& project) {
        Scene::Ptr scene = std::make_shared<Scene>(m_messageBus);
        if(scene == nullptr) {
            m_error = SceneManagerError::MemoryAlloc;
            return false;
        }
        m_activeScene = scene;
        auto path = project -> getPath();
        auto appendPath = combinePath(scenePath, project -> getStartScene());
        auto scenePath = combinePath(path, appendPath);
        m_activeScene -> setPath(scenePath);

        SceneSerializer serializer(m_activeScene);
        if(!serializer.serialize(m_activeScene->getPath())) {
            m_error = SceneManagerError::SceneSerialize;
            return false;
        }

        return true;
    }

    bool SceneManager::load(Project::Ptr&& project) {
        Scene::Ptr scene = std::make_shared<Scene>(m_messageBus);
        if(scene == nullptr) {
            m_error = SceneManagerError::MemoryAlloc;
            return false;
        }
        m_activeScene = scene;
        auto path = project -> getPath();
        auto appendPath = combinePath(scenePath, project->getStartScene());
        auto scenePath = combinePath(path, appendPath);
        m_activeScene -> setPath(scenePath);
        SceneSerializer serializer(m_activeScene);
        if(!serializer.deserialize(m_activeScene->getPath())) {
            m_error = SceneManagerError::SceneDeserialize;
            return false;
        }
        return true;
    }

    bool SceneManager::remove() {
        if(!deleteDirectory(m_activeScene->getPath())) {
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

    bool SceneManager::save(Scene::Ptr&& scene) {
        if(scene == nullptr)
            return false;

        SceneSerializer serializer{scene};
        if(!serializer.serialize(scene->getPath())) {
            m_error = SceneManagerError::SceneSerialize;
            return false;
        }

        return true;
    }

    Project::Ptr SceneManager::getAssociatedProject() const {
        return m_associatedProject;
    }
}
