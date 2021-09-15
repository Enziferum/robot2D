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
#include <functional>

#include "robot2D/Graphics/Color.hpp"
#include "robot2D/Graphics/Texture.hpp"

#include "Vector2.hpp"
#include "Event.hpp"
#include "WindowContext.hpp"

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
        using DrapDropCallback = std::function<void(std::vector<std::string>)>;
    public:
        Window();
        Window(const vec2u& size,
               const std::string& name, WindowContext& windowContext);
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

        void setTitle(const std::string& title);

        const vec2u& getSize();

        /// \brief allow you to set big/small icons to your app
        void setIcon(std::vector<Texture>& icons);

        void setDrapDropCallback(DrapDropCallback&& callback);

        static bool isMousePressed(const Mouse& button);

        static bool isKeyboardPressed(const Key& key);

        void* getRaw() const;

        void setMouseCursorVisible(const bool& flag);
    protected:
        virtual void onResize(const int& w, const int& h);
    private:
        void create();

        float getDeltaTime();
    protected:
        static priv::WindowImpl* m_windowImpl;

        //window settings
        vec2u m_win_size;
        std::string m_name;
        WindowContext m_context;
    };
}
