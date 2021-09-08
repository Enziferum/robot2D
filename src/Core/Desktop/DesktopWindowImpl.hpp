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

#pragma once

#include <queue>

#include <GLFW/glfw3.h>
#include "../WindowImpl.hpp"

#include <robot2D/Core/WindowContext.hpp>

namespace robot2D {
    namespace priv {

        /// \brief our desktop backend is GLFW
        class DesktopWindowImpl: public WindowImpl {
        public:
            DesktopWindowImpl();
            DesktopWindowImpl(const vec2u& size, const std::string& name, WindowContext& context);
            ~DesktopWindowImpl();

            bool pollEvents(Event& event) override;
            void* get_RawWindow() override;

            bool isOpen() const override;
            void setTitle(const std::string& title) const override;

            void clear(const Color &color) override;
            void close() override;
            void display() override;

            bool isMousePressed(const Mouse& button)override;
            bool isKeyboardPressed(const Key& key)override;

            void setIcon(std::vector<robot2D::Texture>& ) override;

            float getDeltaTime() const override;

        private:
            void setup();
            void setup_callbacks();
            void setup_WGL();

        private:
            static void close_callback(GLFWwindow* wnd);
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void cursor_callback(GLFWwindow* window, double xpos, double ypos);
            static void mouseWhell_callback(GLFWwindow* window, double xpos, double ypos);
            static void mouse_callback(GLFWwindow* window, int key, int action, int mods);
            static void size_callback(GLFWwindow* window, int w, int h);
            static void view_callback(GLFWwindow* window, int w, int h);
            static void maximized_callback(GLFWwindow* window, int state);
        private:
            GLFWwindow* m_window;
            std::queue<Event> m_event_queue;

            vec2u m_size;
            std::string m_name;
            WindowContext m_context;
        };
    }
}