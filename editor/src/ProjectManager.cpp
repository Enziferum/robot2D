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

#include <filesystem>
#include <editor/ProjectManager.hpp>
#include <editor/ProjectSerializer.hpp>

namespace editor {

    namespace  {
        namespace fs = std::filesystem;
        const std::string startSceneName = "Scene.scene";
    }

    ProjectManager::ProjectManager(): m_currentProject{nullptr},
    m_error{ProjectManagerError::None} {}

    bool ProjectManager::add(const ProjectDescription& description) {
        Project::Ptr project = std::make_shared<Project>();
        project -> setPath(description.path);
        project -> setName(description.name);
        project -> setEditorVersion("0.1");
        project -> setStartScene(startSceneName);

        std::filesystem::path fullPath(description.path);
        auto fullname = description.name + ".robot2D";
        fullPath.append(fullname);
        ProjectSerializer(project).serialize(fullPath.string());
        m_currentProject = project;

        // directories create

        auto path = project -> getPath();
        fs::path dirPath(path);
        dirPath += fs::path("/assets");

        if(!fs::create_directories(dirPath)) {
            RB_EDITOR_ERROR("Can't create folder {0}", dirPath.string());
            return false;
        }
        dirPath += fs::path("/scenes");
        if(!fs::create_directories(dirPath)) {
            RB_EDITOR_ERROR("Can't create folder {0}", dirPath.string());
            return false;
        }
        dirPath = fs::path(path);
        dirPath += fs::path("/assets/textures");
        if(!fs::create_directories(dirPath)) {
            RB_EDITOR_ERROR("Can't create folder {0}", dirPath.string());
            return false;
        }

        return true;
    }

    bool ProjectManager::load(const ProjectDescription& description) {
        if(m_currentProject) {
            return true;
        }
        m_currentProject = std::make_shared<Project>();
        ProjectSerializer serializer(m_currentProject);
        std::filesystem::path fullPath(description.path);
        auto fullname = description.name + ".robot2D";
        fullPath.append(fullname);
        bool res = serializer.deserialize(fullPath.string());
        return res;
    }

    bool ProjectManager::remove(const ProjectDescription& description) {
        //todo move smart way of deleting
        if(!std::filesystem::remove_all(std::filesystem::path(description.path))){}
        return true;
    }

    Project::Ptr ProjectManager::getCurrentProject() const {
        return m_currentProject;
    }

    const ProjectManagerError& ProjectManager::getError() const {
        return m_error;
    }
}

