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
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include "robot2D/Graphics/Color.h"
#include "robot2D/Graphics/Texture.h"

#include "Vector2.h"
#include "Event.h"

namespace robot2D {
    namespace priv {
        class WindowImpl;
    }

    /**
     * Window - heart of engine, you will use it if you want have onw Rendering part, but
     * you can it fix it with Graphics module easily
     */
    class ROBOT2D_EXPORT_API Window {
    public:
        Window();
        Window(const vec2u& size,
               const std::string& name, const bool& vsync = true);
        ~Window();

        ////
        //// \brief process OS events and puts it to robot2D::Event
        ////
        bool pollEvents(Event& event);

        ////
        //// \brief check if window is opened just now
        ////
        bool isOpen() const;

        ////
        /// \brief process window closing, in next version will allow to set onClose callback
        ///
        void close();

        void clear(const Color& color = Color::Black);

        ///
        /// \brief make swap buffers in render context
        ///
        void display();

        const vec2u& get_size();

        /// \brief allow you to set big/small icons to your app
        void setIcon(std::vector<Texture>& icons);

        // todo rewrite this methods //
        bool getMouseButton(const int& button);
        void setCursorPosition(const vec2f& pos);
        vec2f getCursorPos();

        void* get_RawWindow() const;
    protected:
        virtual void onResize(const int& w, const int& h);
    private:
        void create();

        void setup();
        void setup_callbacks();
        virtual void setup_WGL();


    private:
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_callback(GLFWwindow* window, double xpos, double ypos);
        static void mouseWhell_callback(GLFWwindow* window, double xpos, double ypos);
        static void mouse_callback(GLFWwindow* window, int key, int action, int mods);
        static void size_callback(GLFWwindow* window, int w, int h);
        static void view_callback(GLFWwindow* window, int w, int h);
        static void maximized_callback(GLFWwindow* window, int state);
    protected:
        GLFWwindow* m_window;

        priv::WindowImpl* m_windowImpl;
        std::queue<Event> m_event_queue;

        //window settings
        vec2u m_win_size;
        std::string m_name;
        bool m_vsync;
    };
}
