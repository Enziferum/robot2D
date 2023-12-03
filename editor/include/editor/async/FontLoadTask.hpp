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
#include <editor/Task.hpp>
#include <robot2D/Graphics/Font.hpp>
#include <robot2D/Ecs/Entity.hpp>

namespace editor {
    class FontLoadTask: public ITask {
    public:
        FontLoadTask(ITaskFunction::Ptr function,
                        const std::string& path,
                      robot2D::ecs::Entity entity);
        ~FontLoadTask() override = default;

        void execute() override;
        const robot2D::Font& getFont() const { return m_font; }
        robot2D::ecs::Entity getEntity() const { return std::move(m_entity); }
    private:
        std::string m_fontPath;
        robot2D::ecs::Entity m_entity;
        robot2D::Font m_font;
    };
}