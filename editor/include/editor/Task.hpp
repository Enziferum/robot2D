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
        virtual ~ITask() = 0;

        virtual void execute() = 0;

        virtual void call();
        TaskID getTaskID() const;
    protected:
        friend class TaskQueue;

        ITaskFunction::Ptr m_function;
        TaskID m_taskID{0};
    };
}