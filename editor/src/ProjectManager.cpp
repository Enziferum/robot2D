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

#include <editor/ProjectManager.hpp>
#include <editor/ProjectSerializer.hpp>
#include <editor/FileApi.hpp>

namespace editor {
    ProjectManager::ProjectManager(const Configuration& configuration): m_currentProject{nullptr},
    m_error{ProjectManagerError::None},
    m_configuration{configuration} {}

    bool ProjectManager::add(const ProjectDescription& description) {
        Project::Ptr project = std::make_shared<Project>();
        project -> setPath(description.path);
        project -> setName(description.name);
        {
            auto [status, result] = m_configuration.getValue(ConfigurationKey::Version);
            project->setEditorVersion(result);
        }
        {
            auto [status, result] = m_configuration.getValue(ConfigurationKey::DefaultSceneName);
            project -> setStartScene(result);
        }

        auto [status, extension] = m_configuration.getValue(ConfigurationKey::ProjectExtension);
        auto fullname = description.name + extension;
        auto fullPath = addFilename(description.path, fullname);

        if(!ProjectSerializer(project).serialize(fullPath)) {
            m_error = ProjectManagerError::ProjectSerialize;
            return false;
        }

        auto path = project -> getPath();

        if(!createDirectory(path, "assets")) {
            m_error = ProjectManagerError::CreateFolder;
            return false;
        }

        if(!createDirectory(path, "assets/scenes")){
            m_error = ProjectManagerError::CreateFolder;
            return false;
        }

        if(!createDirectory(path, "assets/textures")) {
            m_error = ProjectManagerError::CreateFolder;
            return false;
        }

        m_currentProject = project;
        return true;
    }

    bool ProjectManager::load(const ProjectDescription& description) {
        if(m_currentProject)
            return true;
        m_currentProject = std::make_shared<Project>();
        ProjectSerializer serializer(m_currentProject);
        auto [status, extension] = m_configuration.getValue(ConfigurationKey::ProjectExtension);
        auto fullname = description.name + extension;
        auto fullPath = combinePath(description.path, fullname);

        if(!serializer.deserialize(fullPath)) {
            m_error = ProjectManagerError::ProjectDeserialize;
            return false;
        }

        return true;
    }

    bool ProjectManager::remove(const ProjectDescription& description) {
        if(!deleteDirectory(description.path)) {
            m_error = ProjectManagerError::RemoveFolder;
            return false;
        }
        return true;
    }

    Project::Ptr ProjectManager::getCurrentProject() const {
        return m_currentProject;
    }

    const ProjectManagerError& ProjectManager::getError() const {
        return m_error;
    }
}

