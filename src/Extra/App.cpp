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

#include <robot2D/Extra/App.h>

namespace robot2D{

    App::App():IStateMachine(),
        lastTime(0.f), deltaTime(0.f)
        {

    }

    //_window(vec2u(size.x, size.y), name, sync)
    App::App(const vec2u& size, const std::string& name, const bool& sync):
    IStateMachine(size, name),
    lastTime(0.f), deltaTime(0.f){
        //c++ hack before c++ 17 to set unused parameter
        (void)(sync);
    }

    void App::run() {
        robot2D::Event event;

        while (m_window.isOpen()){
            handleEvents(event);
            update();
            render();
        }
    }

    void App::handleEvents(robot2D::Event& event) {
        while (m_window.pollEvents(event)) {
            if(m_states.empty())
                return;
            m_states[m_current_state] -> handleEvents(event);
        }
    }

    void App::update() {
        float currentTime = float(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        if(m_states.empty())
            return;
        m_states[m_current_state] -> update(deltaTime);
    }

    void App::render() {
        m_window.clear();
        if(m_states.empty())
            return;
        m_states[m_current_state] -> render();
        m_window.display();
    }

    void App::setIcon(std::vector<Texture>& textures) {
        m_window.setIcon(textures);
    }


}
