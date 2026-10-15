/*********************************************************************
(c) Alex Raag 2026
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

#include <editor/TaskQueue.hpp>
#include <stdexcept>

namespace editor {
    TaskQueue::TaskQueue() {
        m_thread = std::thread{&TaskQueue::threadWork, this};
        m_workerID = m_thread.get_id();
    }

    TaskQueue::~TaskQueue() {
        stop();
    }

    void TaskQueue::threadWork() {
        for(;;) {
            ITask::Ptr task;
            {
                std::unique_lock lock(m_mutex);
                data_cond.wait(lock, [this]() { return m_stopping || !m_inputTasksQueue.empty(); });
                if(m_stopping)
                    return;
                task = std::move(m_inputTasksQueue.front());
                m_inputTasksQueue.pop();
                m_activeTask = task;
                m_statuses.at(task -> getTaskID()).status = TaskStatus::Processing;
            }

            try {
                task -> execute();
                if(task -> m_outcome == TaskOutcome::Pending)
                    task -> m_outcome = TaskOutcome::Succeeded;
            }
            catch(const std::exception& error) {
                task -> m_exception = std::current_exception();
                task -> fail(error.what());
            }
            catch(...) {
                task -> m_exception = std::current_exception();
                task -> fail("Unknown exception in task execution");
            }

            if(task -> getOutcome() == TaskOutcome::Failed) {
                RB_EDITOR_ERROR("[TaskQueue] Task failed; task_id={}; reason='{}'",
                                task -> getTaskID(), task -> getErrorMessage());
            }
            {
                std::lock_guard lock(m_mutex);
                auto& info = m_statuses.at(task -> getTaskID());
                info.exceptionPtr = task -> getException();
                if(m_stopping) {
                    task -> markCancelled();
                    info.status = TaskStatus::Cancelled;
                }
                else {
                    switch(task -> getOutcome()) {
                        case TaskOutcome::Succeeded: info.status = TaskStatus::Done; break;
                        case TaskOutcome::Failed: info.status = TaskStatus::Failed; break;
                        default: info.status = TaskStatus::Cancelled; break;
                    }
                    m_outputTasksQueue.push(task);
                }
                m_activeTask.reset();
            }
            // Results discarded during shutdown are destroyed before the worker exits.
            task.reset();
        }
    }

    void TaskQueue::process() {
        // Serialize callback delivery with stop(), including stop() called by a callback.
        std::lock_guard callbackLock(m_callbackMutex);
        ITask::Ptr task;
        {
            std::lock_guard lock(m_mutex);
            if(m_stopping || m_outputTasksQueue.empty())
                return;
            task = std::move(m_outputTasksQueue.front());
            m_outputTasksQueue.pop();
            ++m_activeCallbacks;
        }
        try {
            task -> call();
        }
        catch(...) {
            const auto exception = std::current_exception();
            {
                std::lock_guard lock(m_mutex);
                m_statuses.at(task -> getTaskID()) = {TaskStatus::Failed, exception};
            }
            RB_EDITOR_ERROR("[TaskQueue] Completion callback threw; task_id={}", task -> getTaskID());
        }
        {
            std::lock_guard lock(m_mutex);
            --m_activeCallbacks;
        }
    }

    void TaskQueue::discard(std::queue<ITask::Ptr>& tasks) {
        // m_mutex is held by the caller. Destruction happens after the queues are detached.
        auto pending = tasks;
        while(!pending.empty()) {
            auto& task = pending.front();
            task -> markCancelled();
            m_statuses.at(task -> getTaskID()).status = TaskStatus::Cancelled;
            pending.pop();
        }
    }

    void TaskQueue::requestStop() {
        // A concurrent stop() must also wait until the cancellation hook and discarded results are released.
        std::lock_guard requestLock(m_requestMutex);
        std::queue<ITask::Ptr> waiting;
        std::queue<ITask::Ptr> ready;
        ITask::Ptr active;
        {
            std::lock_guard lock(m_mutex);
            if(m_stopping)
                return;
            m_stopping = true;
            discard(m_inputTasksQueue);
            discard(m_outputTasksQueue);
            waiting.swap(m_inputTasksQueue);
            ready.swap(m_outputTasksQueue);
            active = m_activeTask;
        }
        data_cond.notify_all();
        if(active)
            active -> requestCancellation();
        RB_EDITOR_INFO("[TaskQueue] Stop requested; waiting_discarded={}; results_discarded={}; active={}",
                       waiting.size(), ready.size(), static_cast<bool>(active));
    }

    void TaskQueue::stop() {
        if(std::this_thread::get_id() == m_workerID)
            throw std::logic_error("TaskQueue::stop() cannot join its own worker; use requestStop()");
        requestStop();
        std::lock_guard callbackLock(m_callbackMutex);
        std::lock_guard stopLock(m_stopMutex);
        if(m_thread.joinable())
            m_thread.join();
        clear();
    }

    TaskInfo TaskQueue::getTaskInfo(TaskID taskId) const {
        std::lock_guard lock(m_mutex);
        auto found = m_statuses.find(taskId);
        return found == m_statuses.end() ? TaskInfo{TaskStatus::Bad} : found -> second;
    }

    TaskStatus TaskQueue::getStatus(TaskID taskId) {
        return getTaskInfo(taskId).status;
    }

    bool TaskQueue::hasPendingTasks() const {
        std::lock_guard lock(m_mutex);
        return m_activeTask || m_activeCallbacks || !m_inputTasksQueue.empty() || !m_outputTasksQueue.empty();
    }

    void TaskQueue::clear() {
        std::queue<ITask::Ptr> waiting;
        std::queue<ITask::Ptr> ready;
        {
            std::lock_guard lock(m_mutex);
            discard(m_inputTasksQueue);
            discard(m_outputTasksQueue);
            waiting.swap(m_inputTasksQueue);
            ready.swap(m_outputTasksQueue);
        }
    }
}
