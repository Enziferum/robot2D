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
#include <memory>

#include <robot2D/Core/Event.hpp>
#include "IStateMachine.hpp"


namespace robot2D{
    class ROBOT2D_EXPORT_API State{
    public:
        using Ptr = std::shared_ptr<State>;
    public:
        State(IStateMachine& machine);
        virtual ~State() = 0;

        virtual void handleEvents(const Event&) = 0;
        virtual void update(float dt) = 0;
        virtual void render() = 0;

        void setActive(const bool& flag);
        bool isActive() const;
    protected:
        IStateMachine& m_machine;
        RenderWindow& m_window;

        bool m_active;
    };
}