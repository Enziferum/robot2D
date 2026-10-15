/*********************************************************************
(c) Alex Raag 2023
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

#include <vector>

#include "ProjectDescription.hpp"
#include "Errors.hpp"
#include "Configuration.hpp"

namespace editor {
    class EditorCache {
    public:
        EditorCache(const Configuration& configuration);
        EditorCache(const EditorCache& other) = delete;
        EditorCache& operator=(const EditorCache& other) = delete;
        EditorCache(EditorCache&& other) = delete;
        EditorCache& operator=(EditorCache&& other) = delete;
        ~EditorCache() = default;

        bool parseCache(const std::string& path);
        bool isShowInspector() const;
        bool showInspectorOnStartup() const;
        bool setShowInspector(const bool& flag);

        const EditorCacheError& getError() const;

        bool addProject(const ProjectDescription& description);
        bool removeProject(const ProjectDescription& description);
        bool loadProject(const ProjectDescription& description);
        const std::vector<ProjectDescription>& getProjects() const;

        const ProjectDescription& getCurrentProject() const;
    private:
        bool save(const std::vector<ProjectDescription>& projects,
                  const ProjectDescription& current, bool showInspector);
        bool backup(const std::string& suffix);
        bool recoverInvalidCache(const char* reason);
    private:
        bool m_showInspector = true;
        bool m_writeEnabled = false;
        std::string m_cachePath;
        EditorCacheError m_cacheError = EditorCacheError::None;

        ProjectDescription m_currentDescription;

        std::vector<ProjectDescription> m_cacheDescriptions;
    };
}
