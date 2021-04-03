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

#include <unordered_map>
#include <memory>
#include <cassert>
#include <string>

#include "State.h"
#include "IStateMachine.h"

namespace robot2D{

    class State;
    class App final: public IStateMachine{
    public:
        App();
        App(const vec2u& size, const std::string& name, const bool& sync);
        ~App() = default;

        template<typename T, typename ... Args>
        void register_state(const int id, Args&& ... args);

        void run();
        void setIcon(std::vector<Texture>& textures);
    private:
        void handleEvents(robot2D::Event&);
        void update();
        void render();
    private:
        vec2u m_size;
        std::string m_name;
        bool m_vsync;

        float lastTime;
        float deltaTime;
    };

    template<class T, typename ... Args>
    void App::register_state(const int id, Args&& ... args) {
        if(!std::is_base_of<State, T>::value)
            return;
        State::Ptr ptr = std::make_shared<T>(std::forward<Args>(args) ...);

        if(ptr == nullptr)
            return;

        m_states.insert(std::pair<int, State::Ptr>(id, ptr));
        m_current_state = id;
    }

}
