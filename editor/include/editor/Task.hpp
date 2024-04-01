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
#include <cstdint>
#include "TaskFunction.hpp"

namespace editor {
    using TaskID = std::uint32_t;

    class ITask {
    public:
        using Ptr = std::shared_ptr<ITask>;
    public:
        explicit ITask(ITaskFunction::Ptr function);
        ITask(const ITask& other) = delete;
        ITask& operator=(const ITask& other) = delete;
        ITask(ITask&& other) = delete;
        ITask& operator=(ITask&& other) = delete;
        virtual ~ITask() = 0;

        virtual void execute() = 0;

        virtual void call();
        TaskID getTaskID() const;
    protected:
        friend class TaskQueue;

        ITaskFunction::Ptr m_function;
        TaskID m_taskID{ 0 };
    };
}