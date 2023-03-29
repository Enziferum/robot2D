#include <editor/TaskQueue.hpp>

namespace editor {
    TaskQueue::TaskQueue(): m_inputTasksQueue{},
                            m_outputTasksQueue{},
                            m_currentId{0} {
        try {
            m_thread = std::thread{&TaskQueue::threadWork, this};
            m_running.store(true, std::memory_order::memory_order_relaxed);
        }
        catch(...) {
            m_running.store(false, std::memory_order::memory_order_relaxed);
        }
    }

    TaskQueue::~TaskQueue() {
        m_running.store(false, std::memory_order::memory_order_relaxed);

        if(m_thread.joinable())
            m_thread.join();
    }

    void TaskQueue::threadWork() {
        while (m_running.load(std::memory_order::memory_order_relaxed)) {
            using namespace std::chrono_literals;

            std::unique_lock<std::mutex> lock(m_inputMutex);
            data_cond.wait(lock, [this]{ return !m_inputTasksQueue.empty(); });

            auto task = m_inputTasksQueue.front();
            m_inputTasksQueue.pop();
            if(!task)
                continue;

            task -> execute();

            {
                std::lock_guard<std::mutex> outLock{m_outputMutex};
                m_outputTasksQueue.push(task);
            }
        }
    }

    void TaskQueue::process() {
        ITask::Ptr task{nullptr};
        {
            std::lock_guard<std::mutex> outLock{m_outputMutex};
            if(!m_outputTasksQueue.empty()) {
                task = m_outputTasksQueue.front();
                m_outputTasksQueue.pop();
            }
        }
        if(task)
            task -> call();
    }
}