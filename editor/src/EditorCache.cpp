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

#include <fstream>
#include <sstream>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <editor/EditorCache.hpp>

namespace editor {
    EditorCache::EditorCache(const Configuration& configuration): m_showInspector(false),
                                m_cacheDescriptions(),
                                m_configuration(configuration) {}

    bool EditorCache::parseCache(const std::string& path) {
        m_cachePath = path;
        if(!std::filesystem::exists(path)) {
            m_cacheError = EditorCacheError::NoCacheFile;
            createCacheFile(path);
            return false;
        }

        std::ifstream ifstream(path);
        if(!ifstream.is_open())
            return false;
        std::stringstream sstr;
        sstr << ifstream.rdbuf();

        YAML::Node data = YAML::Load(sstr.str());
        if(!data["Robot2D Editor"]) {
            m_cacheError = EditorCacheError::EditorTagNone;
            return false;
        }
        if(data["ShowInspector"])
            m_showInspector = data["ShowInspector"].as<bool>();

        auto projects = data["Projects"];
        if(!projects) {
            m_cacheError = EditorCacheError::ProjectsTagNone;
            return false;
        }

        for(auto it: projects) {
            ProjectDescription currentProject;
            currentProject.path = it["Path"].as<std::string>();
            currentProject.name = it["Name"].as<std::string>();
            m_cacheDescriptions.emplace_back(currentProject);
        }

        if(data["CurrentProject"]) {
            m_currentDescription.path = data["CurrentProject"].as<std::string>();
        }

        return true;
    }

    void EditorCache::createCacheFile(const std::string& path) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        auto [status, result] = m_configuration.getValue(ConfigurationKey::Version);
        out << YAML::Key << "Robot2D Editor" << YAML::Value << result;
        out << YAML::EndMap;

        std::ofstream ofstream(path);
        if(!ofstream.is_open()) {
            m_cacheError = EditorCacheError::NoCacheFile;
            return;
        }

        ofstream << out.c_str();
        ofstream.close();
    }

    bool EditorCache::loadProject(const ProjectDescription& description) {
        std::ifstream ifstream(m_cachePath);
        if(!ifstream.is_open()) {
            return false;
        }
        std::stringstream sstr;
        sstr << ifstream.rdbuf();
        ifstream.close();

        YAML::Node data = YAML::Load(sstr.str());
        YAML::Emitter out;
        if(!data["Robot2D Editor"]) {
            m_cacheError = EditorCacheError::EditorTagNone;
            return false;
        }

        data["CurrentProject"] = description.path;
        data["ShowInspector"] = m_showInspector;

        std::ofstream ofstream(m_cachePath);
        ofstream << data;
        ofstream.close();
        return true;
    }

    bool EditorCache::addProject(const ProjectDescription& description) {
        std::ifstream ifstream(m_cachePath);
        if(!ifstream.is_open()) {
            return false;
        }
        std::stringstream sstr;
        sstr << ifstream.rdbuf();
        ifstream.close();

        YAML::Node data = YAML::Load(sstr.str());
        YAML::Emitter out;
        if(!data["Robot2D Editor"]) {
            m_cacheError = EditorCacheError::EditorTagNone;
            return false;
        }

        YAML::Node projectsNode(YAML::NodeType::Map);
        projectsNode["Path"] = (description.path);
        projectsNode["Name"] = (description.name);
        data["Projects"].push_back(projectsNode);
        data["CurrentProject"] = description.path;
        data["ShowInspector"] = m_showInspector;

        std::ofstream ofstream(m_cachePath);
        ofstream << data;
        ofstream.close();
        return true;
    }

    bool EditorCache::removeProject(const ProjectDescription& description) {
        std::ifstream ifstream(m_cachePath);
        if(!ifstream.is_open())
            return false;

        std::stringstream sstr;
        sstr << ifstream.rdbuf();
        ifstream.close();

        YAML::Node data = YAML::Load(sstr.str());
        YAML::Emitter out;
        if(!data["Robot2D Editor"]) {
            m_cacheError = EditorCacheError::EditorTagNone;
            return false;
        }
        if(!data["Projects"]) {
            m_cacheError = EditorCacheError::ProjectsTagNone;
            return false;
        }
        if(data["CurrentProject"]) {
            m_currentDescription.path = data["CurrentProject"].as<std::string>();
        }

        m_cacheDescriptions.clear();

        for(auto project: data["Projects"]) {
            ProjectDescription it;
            it.path = project["Path"].as<std::string>();
            it.name = project["Name"].as<std::string>();
            if (it.path == description.path)
                continue;
            m_cacheDescriptions.emplace_back(std::move(it));
        }

        auto version = data["Robot2D Editor"].as<std::string>();
        out << YAML::BeginMap;
        out << YAML::Key << "Robot2D Editor" << YAML::Value << version;
        out << YAML::Key << "Projects" << YAML::Value << YAML::BeginSeq;
        for(const auto& it: m_cacheDescriptions) {
            out << YAML::BeginMap;
            out << YAML::Key << "Path" << YAML::Value << it.path;
            out << YAML::Key << "Name" << YAML::Value << it.name;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        if(description.path != m_currentDescription.path)
            out << YAML::Key << "CurrentProject" << YAML::Value << m_currentDescription.path;
        out << YAML::Key << "ShowInspector" << YAML::Value << m_showInspector;
        out << YAML::EndMap;

        std::ofstream ofstream(m_cachePath);
        ofstream << out.c_str();
        ofstream.close();
        return true;
    }

    bool EditorCache::isShowInspector() const {
        return m_showInspector or m_currentDescription.empty();
    }

    const EditorCacheError& EditorCache::getError() const {
        return m_cacheError;
    }

    std::vector<ProjectDescription>& EditorCache::getProjects() {
        return m_cacheDescriptions;
    }

    const ProjectDescription& EditorCache::getCurrentProject() const {
        m_currentDescription.name = "Project";
        return m_currentDescription;
    }

    void EditorCache::setShowInspector(const bool& flag) {
        m_showInspector = flag;
    }
}

