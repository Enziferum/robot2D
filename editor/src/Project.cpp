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

#include <cassert>
#include <editor/Project.hpp>

namespace editor {

    Project::Project():
    m_name{"Untitled Project"}, m_path{""}, m_startSceneName("") {}


    void Project::setName(const std::string& name) {
        if(name.empty())
            return;
        m_name = name;
    }

    void Project::setPath(const std::string& path) {
        if(path.empty())
            return;
        m_path = path;
    }

    const std::string& Project::getName() {
        return m_name;
    }

    const std::string& Project::getPath() {
        return m_path;
    }

    void Project::setEditorVersion(const std::string& version) {
        m_editorVersion = version;
    }

    void Project::setStartScene(const std::string& name) {
        m_startSceneName = name;
    }

    const std::string& Project::getStartScene() {
        return m_startSceneName;
    }

    const std::string& Project::getEditorVersion() {
        return m_editorVersion;
    }
}