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

#include <robot2D/Graphics/RenderWindow.h>

namespace robot2D{
    class State;
    class IStateMachine{
    public:
        IStateMachine();
        virtual ~IStateMachine();

        void pushState(const int& state);
        void popState();
        robot2D::RenderWindow& getWindow(){
            return m_window;
        }

        void setCurrent(const unsigned int& id);
    protected:
        RenderWindow m_window;
        std::unordered_map<unsigned int,
                std::shared_ptr<State>> m_states;
        unsigned int m_current_state = -1;
    };


}