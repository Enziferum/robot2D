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

#include <string>
#include <memory>
#include "editor/Errors.hpp"

namespace editor {

    class Project;
    class ProjectSerializer {
    public:
        ProjectSerializer(std::shared_ptr<Project> project);
        ProjectSerializer(const ProjectSerializer& other) = delete;
        ProjectSerializer& operator=(const ProjectSerializer& other) = delete;
        ProjectSerializer(ProjectSerializer&& other) = delete;
        ProjectSerializer& operator=(ProjectSerializer&& other) = delete;
        ~ProjectSerializer() = default;

        bool serialize(const std::string& path);
        bool deserialize(const std::string& path);

        ProjectSerializerError getError() const;
    private:
        std::shared_ptr<Project> m_project;
        ProjectSerializerError m_error;
    };
}
