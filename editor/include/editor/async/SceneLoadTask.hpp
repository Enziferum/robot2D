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
#include <string>

#include <editor/SceneManager.hpp>
#include <editor/EditorLogic.hpp>
#include <editor/Task.hpp>

namespace editor {
    /// TODO(a.raag): Move in RB-40 to robot2DAsync library
    class SceneLoadTask final: public ITask {
    public:
        SceneLoadTask(ITaskFunction::Ptr function,
                      SceneManager& sceneManager,
                      Project::Ptr project,
                      std::string path,
                      EditorLogic* logic);
        SceneLoadTask(const SceneLoadTask& other) = delete;
        SceneLoadTask& operator=(const SceneLoadTask& other) = delete;
        SceneLoadTask(SceneLoadTask&& other) = delete;
        SceneLoadTask& operator=(SceneLoadTask&& other) = delete;
        ~SceneLoadTask() override = default;


        void execute() override;
        EditorLogic* getLogic() const { return m_logic;}
    private:
        SceneManager& m_sceneManager;
        Project::Ptr m_project;
        std::string m_path;
        EditorLogic* m_logic;
    };
} // namespace editor