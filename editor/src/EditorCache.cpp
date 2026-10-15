/*********************************************************************
(c) Alex Raag 2026
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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <robot2D/Util/Logger.hpp>
#include <editor/EditorCache.hpp>
#include <editor/BuildInfo.hpp>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

namespace editor {

    namespace {
        constexpr unsigned cacheVersion = 1;
        namespace fs = std::filesystem;

        bool validString(const std::string& value) {
            return !value.empty() && value.find('\0') == std::string::npos;
        }

        bool validProject(const ProjectDescription& description) {
            return validString(description.name) && validString(description.path);
        }

        bool samePath(const std::string& left, const std::string& right) {
            if(left.empty() || right.empty())
                return left == right;
            const auto a = fs::u8path(left).lexically_normal();
            const auto b = fs::u8path(right).lexically_normal();
            if(a == b)
                return true;
            std::error_code error;
            return fs::equivalent(a, b, error) && !error;
        }

        fs::path unusedSibling(const fs::path& path, const std::string& suffix, std::error_code& error) {
            auto candidate = path;
            candidate += suffix;
            for(unsigned i = 1; fs::exists(candidate, error) && !error; ++i) {
                candidate = path;
                candidate += suffix + "." + std::to_string(i);
            }
            return candidate;
        }

        bool replaceFile(const fs::path& temporary, const fs::path& destination) {
#ifdef _WIN32
            return MoveFileExW(temporary.c_str(), destination.c_str(),
                               MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
            std::error_code error;
            fs::rename(temporary, destination, error);
            return !error;
#endif
        }
    }

    EditorCache::EditorCache(const Configuration&) {}

    bool EditorCache::parseCache(const std::string& path) {
        m_showInspector = true;
        m_currentDescription = {};
        m_cacheDescriptions.clear();
        m_cacheError = EditorCacheError::None;
        m_writeEnabled = false;
        m_cachePath.clear();
        if(path.empty()) {
            m_cacheError = EditorCacheError::ReadFailed;
            return false;
        }

        try {
            std::error_code error;
            const auto cachePath = fs::absolute(fs::u8path(path), error).lexically_normal();
            if(error) {
                m_cacheError = EditorCacheError::ReadFailed;
                return false;
            }
            m_cachePath = cachePath.u8string();
            const bool exists = fs::exists(cachePath, error);
            if(error) {
                m_cacheError = EditorCacheError::ReadFailed;
                return false;
            }
            if(!exists) {
                m_writeEnabled = true;
                if(save(m_cacheDescriptions, m_currentDescription, m_showInspector))
                    return true;
                m_writeEnabled = false;
                return false;
            }
            if(!fs::is_regular_file(cachePath, error) || error) {
                m_cacheError = EditorCacheError::ReadFailed;
                return false;
            }
            std::ifstream input(cachePath, std::ios::binary);
            if(!input.is_open()) {
                m_cacheError = EditorCacheError::ReadFailed;
                return false;
            }
            std::stringstream contents;
            contents << input.rdbuf();
            if(input.bad()) {
                m_cacheError = EditorCacheError::ReadFailed;
                return false;
            }
            input.close();
            const auto data = YAML::Load(contents.str());
            if(!data.IsMap())
                return recoverInvalidCache("Expected a YAML map");

            const auto versionNode = data["CacheVersion"];
            const auto version = versionNode ? versionNode.as<unsigned long long>() : 0ULL;
            if(version > cacheVersion) {
                m_cacheError = EditorCacheError::UnsupportedVersion;
                return false;
            }
            const bool legacy = version == 0;
            if(legacy && !data["Robot2D Editor"].IsScalar())
                return recoverInvalidCache("Missing legacy editor marker");
            if((data["EditorVersion"] && !data["EditorVersion"].IsScalar()) ||
               (data["EditorBuild"] && !data["EditorBuild"].IsScalar()))
                return recoverInvalidCache("Editor metadata must be scalar values");

            const auto projectsNode = data["Projects"];
            if((projectsNode && !projectsNode.IsSequence()) || (!legacy && !projectsNode))
                return recoverInvalidCache("Projects must be a sequence");
            bool showInspector = true;
            if(data["ShowInspector"])
                showInspector = data["ShowInspector"].as<bool>();
            std::vector<ProjectDescription> projects;
            if(projectsNode) {
                projects.reserve(projectsNode.size());
                for(const auto& item: projectsNode) {
                    if(!item.IsMap() || !item["Name"].IsScalar() || !item["Path"].IsScalar())
                        return recoverInvalidCache("Each project must have scalar Name and Path fields");
                    ProjectDescription description;
                    description.name = item["Name"].as<std::string>();
                    description.path = item["Path"].as<std::string>();
                    if(!validProject(description))
                        return recoverInvalidCache("Project name or path is empty or contains a null character");
                    const auto found = std::find_if(projects.begin(), projects.end(), [&](const auto& project) {
                        return samePath(project.path, description.path);
                    });
                    if(found == projects.end())
                        projects.emplace_back(std::move(description));
                }
            }
            ProjectDescription current;
            if(data["CurrentProject"]) {
                const auto currentPath = data["CurrentProject"].as<std::string>();
                const auto found = std::find_if(projects.begin(), projects.end(), [&](const auto& project) {
                    return samePath(project.path, currentPath);
                });
                if(found != projects.end())
                    current = *found;
            }

            // Keep a successfully read legacy cache usable even if migration cannot be saved.
            m_cacheDescriptions = std::move(projects);
            m_currentDescription = std::move(current);
            m_showInspector = showInspector;
            if(legacy && !backup(".legacy.bak")) {
                m_showInspector = true;
                return false;
            }
            m_writeEnabled = true;
            if(legacy && !save(m_cacheDescriptions, m_currentDescription, m_showInspector)) {
                m_writeEnabled = false;
                m_showInspector = true;
                return false;
            }
            return true;
        }
        catch(const YAML::Exception& error) {
            return recoverInvalidCache(error.what());
        }
        catch(const fs::filesystem_error&) {
            m_cacheError = EditorCacheError::ReadFailed;
            return false;
        }
    }

    bool EditorCache::backup(const std::string& suffix) {
        std::error_code error;
        const auto source = fs::u8path(m_cachePath);
        const auto destination = unusedSibling(source, suffix, error);
        if(error || !fs::copy_file(source, destination, fs::copy_options::none, error) || error) {
            m_cacheError = EditorCacheError::BackupFailed;
            return false;
        }
        return true;
    }

    bool EditorCache::recoverInvalidCache(const char* reason) {
        RB_EDITOR_WARN("Invalid editor cache '{0}': {1}", m_cachePath, reason);
        m_showInspector = true;
        m_currentDescription = {};
        m_cacheDescriptions.clear();
        m_writeEnabled = false;
        if(!backup(".invalid.bak"))
            return false;
        m_writeEnabled = true;
        if(save(m_cacheDescriptions, m_currentDescription, m_showInspector))
            m_cacheError = EditorCacheError::InvalidCache;
        else
            m_writeEnabled = false;
        return false;
    }

    bool EditorCache::save(const std::vector<ProjectDescription>& projects,
                           const ProjectDescription& current, bool showInspector) {
        // Unsupported or unreadable files are preserved; the editor can still work for this session.
        if(!m_writeEnabled)
            return true;
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "CacheVersion" << YAML::Value << cacheVersion;
        out << YAML::Key << "EditorVersion" << YAML::Value << buildInfo::getVersion();
        out << YAML::Key << "EditorBuild" << YAML::Value << buildInfo::buildID;
        out << YAML::Key << "ShowInspector" << YAML::Value << showInspector;
        if(!current.path.empty())
            out << YAML::Key << "CurrentProject" << YAML::Value << current.path;
        out << YAML::Key << "Projects" << YAML::Value << YAML::BeginSeq;
        for(const auto& project: projects) {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << project.name;
            out << YAML::Key << "Path" << YAML::Value << project.path;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq << YAML::EndMap;
        if(!out.good()) {
            m_cacheError = EditorCacheError::WriteFailed;
            return false;
        }

        std::error_code error;
        const auto destination = fs::u8path(m_cachePath);
        const auto temporary = unusedSibling(destination, ".tmp", error);
        if(error) {
            m_cacheError = EditorCacheError::WriteFailed;
            return false;
        }
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if(output.is_open()) {
            output << out.c_str() << '\n';
            output.flush();
            output.close();
            if(output && replaceFile(temporary, destination)) {
                m_cacheError = EditorCacheError::None;
                return true;
            }
        }
        fs::remove(temporary, error);
        m_cacheError = EditorCacheError::WriteFailed;
        return false;
    }

    bool EditorCache::addProject(const ProjectDescription& description) {
        if(!validProject(description)) {
            m_cacheError = EditorCacheError::InvalidProject;
            return false;
        }
        auto projects = m_cacheDescriptions;
        const auto found = std::find_if(projects.begin(), projects.end(), [&](const auto& project) {
            return samePath(project.path, description.path);
        });
        const auto current = description;
        if(found == projects.end())
            projects.push_back(current);
        else
            *found = current;
        if(!save(projects, current, m_showInspector))
            return false;
        m_currentDescription = current;
        m_cacheDescriptions.swap(projects);
        return true;
    }

    bool EditorCache::loadProject(const ProjectDescription& description) {
        return addProject(description);
    }

    bool EditorCache::removeProject(const ProjectDescription& description) {
        if(!validProject(description)) {
            m_cacheError = EditorCacheError::InvalidProject;
            return false;
        }
        auto projects = m_cacheDescriptions;
        projects.erase(std::remove_if(projects.begin(), projects.end(), [&](const auto& project) {
            return samePath(project.path, description.path);
        }), projects.end());
        auto current = m_currentDescription;
        if(samePath(current.path, description.path))
            current = {};
        if(!save(projects, current, m_showInspector))
            return false;
        m_currentDescription = std::move(current);
        m_cacheDescriptions.swap(projects);
        return true;
    }

    bool EditorCache::setShowInspector(const bool& flag) {
        if(!save(m_cacheDescriptions, m_currentDescription, flag))
            return false;
        m_showInspector = flag;
        return true;
    }

    bool EditorCache::isShowInspector() const {
        return m_showInspector || m_currentDescription.empty();
    }

    bool EditorCache::showInspectorOnStartup() const {
        return m_showInspector;
    }

    const EditorCacheError& EditorCache::getError() const {
        return m_cacheError;
    }

    const std::vector<ProjectDescription>& EditorCache::getProjects() const {
        return m_cacheDescriptions;
    }

    const ProjectDescription& EditorCache::getCurrentProject() const {
        return m_currentDescription;
    }

} // namespace editor
