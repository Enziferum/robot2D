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
#include <yaml-cpp/yaml.h>
#include <editor/ProjectSerializer.hpp>
#include <editor/Project.hpp>

namespace editor {

    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project): m_project{project} {}

    bool ProjectSerializer::serialize(const std::string& path) {
        if(m_project == nullptr)
            return false;

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << m_project -> getName();
        out << YAML::Key << "Path" << YAML::Value << m_project -> getPath();
        out << YAML::Key << "Editor Version" << YAML::Value << m_project -> getEditorVersion();
        out << YAML::Key << "Start Scene" << YAML::Value << m_project -> getStartScene();
        out << YAML::EndMap;

        std::ofstream ofstream(path);
        if(!ofstream.is_open())
            return false;

        ofstream << out.c_str();
        ofstream.close();

        return true;
    }

    bool ProjectSerializer::deserialize(const std::string& path) {
        if(m_project == nullptr)
            return false;

        std::ifstream ifstream(path);
        if(!ifstream.is_open())
            return false;
        std::stringstream sstr;
        sstr << ifstream.rdbuf();

        YAML::Node data = YAML::Load(sstr.str());
        if(!data["Name"])
            return false;

        auto parsePath = data["Path"].as<std::string>();
        m_project -> setPath(parsePath);
        m_project -> setName(data["Name"].as<std::string>());
        m_project -> setEditorVersion(data["Editor Version"].as<std::string>());
        m_project -> setStartScene(data["Start Scene"].as<std::string>());

        return true;
    }
}
