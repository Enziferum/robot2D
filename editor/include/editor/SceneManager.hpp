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
#pragma once

#include <unordered_map>
#include <vector>
#include <robot2D/Core/MessageBus.hpp>

#include "Scene.hpp"
#include "Errors.hpp"
#include "Project.hpp"

namespace editor {
    class SceneManager {
    public:
        SceneManager(robot2D::MessageBus& messageBus);
        SceneManager(const SceneManager& other) = delete;
        SceneManager& operator=(const SceneManager& other) = delete;
        SceneManager(SceneManager&& other) = delete;
        SceneManager& operator=(SceneManager&& other) = delete;
        ~SceneManager() = default;

        bool add(Project::Ptr&& project);
        bool add(Project::Ptr&& project, const std::string& path);

        bool load(const Project::Ptr project, std::string path = "");
        bool remove();
        bool save(Scene::Ptr&& scene);

        Project::Ptr getAssociatedProject() const;
        Scene::Ptr getActiveScene() const;
        const SceneManagerError& getError() const;
    private:
        void loadAssetByEntity(SceneEntity& entity);
    private:
        robot2D::MessageBus& m_messageBus;
        Project::Ptr m_associatedProject;
        Scene::Ptr m_activeScene;
        SceneManagerError m_error;
    };

} // namespace editor