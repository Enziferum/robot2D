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
#include <memory>

namespace editor {
    class ITaskFunction {
    public:
        using Ptr = std::shared_ptr<ITaskFunction>;
        virtual ~ITaskFunction() = 0;

        virtual void execute(void* buffer) = 0;
    };

    template<typename T, typename Func>
    class TaskFunction: public ITaskFunction {
    public:
        TaskFunction(Func&& func): m_func{std::forward<Func>(func)} {}
        TaskFunction(const TaskFunction& other) = delete;
        TaskFunction& operator=(const TaskFunction& other) = delete;
        TaskFunction(TaskFunction&& other) = delete;
        TaskFunction& operator=(TaskFunction&& other) = delete;
        ~TaskFunction() override = default;

        void execute(void* buffer) override {
            if(!buffer) {
                // TODO log
                return;
            }
            const T* funcData = static_cast<T*>(buffer);
            if(funcData)
                m_func(*funcData);
        }
    private:
        Func m_func;
    };
}