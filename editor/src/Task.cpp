#include <editor/Task.hpp>

namespace editor {
    ITask::ITask(ITaskFunction::Ptr function):
            m_function{function} {}

    ITask::~ITask() {}

    void ITask::call() {
        if(!m_function)
            return;
        m_function -> execute(static_cast<void*>(this));
    }

    TaskID ITask::getTaskID() const { return m_taskID; }
}