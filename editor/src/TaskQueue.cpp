#include <editor/TaskQueue.hpp>

namespace editor {
    TaskQueue::TaskQueue(): m_inputTasksQueue{},
                            m_outputTasksQueue{},
                            m_currentId{0} {
        m_thread = std::thread{&TaskQueue::threadWork, this};
    }

    TaskQueue::~TaskQueue() {
        if(m_thread.joinable())
            m_thread.join();
    }

    void TaskQueue::threadWork() {
        bool m_running = true;
        while (m_running) {
            using namespace std::chrono_literals;

            std::unique_lock<std::mutex> lock(m_inputMutex);
            data_cond.wait(lock, [this]{ return !m_inputTasksQueue.empty(); });

            auto task = m_inputTasksQueue.front();
            m_inputTasksQueue.pop();
            if(!task)
                continue;

            task -> execute();


      //      std::this_thread::sleep_for(5000ms);

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