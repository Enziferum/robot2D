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
#include <list>

#include "ProjectDescription.hpp"
#include "Errors.hpp"
#include "Configuration.hpp"

namespace editor {
    class EditorCache {
    public:
        EditorCache(const Configuration& configuration);
        ~EditorCache() = default;

        bool parseCache(const std::string& path);
        bool isShowInspector() const;
        void setShowInspector(const bool& flag);

        const EditorCacheError& getError() const;

        bool addProject(const ProjectDescription& description);
        bool removeProject(const ProjectDescription& description);
        bool loadProject(const ProjectDescription& description);
        const std::vector<ProjectDescription>& getProjects() const;

        const ProjectDescription& getCurrentProject() const;
    private:
        void createCacheFile(const std::string& path);
    private:
        bool m_showInspector;
        std::string m_cachePath;
        EditorCacheError m_cacheError;

        using PathPair = std::pair<std::string, std::string>;
        mutable ProjectDescription m_currentDescription;

        std::vector<ProjectDescription> m_cacheDescriptions;
        Configuration m_configuration;
    };
}