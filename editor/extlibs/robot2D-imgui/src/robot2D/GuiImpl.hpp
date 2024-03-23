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

#include <array>
#include <memory>

#include <imgui/imgui.h>
#include <robot2D/Graphics/RenderWindow.hpp>
#include "Render.hpp"

namespace robot2D::priv {

    class GuiImpl {
    public:
        GuiImpl();
        GuiImpl(const GuiImpl& other)=delete;
        GuiImpl& operator=(const GuiImpl& other)=delete;
        GuiImpl(GuiImpl&& other)=delete;
        GuiImpl& operator=(GuiImpl&& other)=delete;
        ~GuiImpl() noexcept;

        void setup(robot2D::Window& window,
                   const std::string& customFontPath = "", std::vector<std::string>&& customIconsPaths = {});
        void setCustomFont();
        void handleEvents(const robot2D::Event& event);
        void update(float dt);
        void render();
    private:
        void shutdown();
        void updateMouseCursor();
    private:
        robot2D::Window* m_window;
        std::array<robot2D::Cursor, ImGuiMouseCursor_COUNT> m_cursors;
        std::array<bool, 3> m_mousePressed;

        bool m_windowHasFocus;
        GuiRender m_render;
    };
} // namespace robot2D