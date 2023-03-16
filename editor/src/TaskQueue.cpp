#include <editor/TaskQueue.hpp>

namespace editor {
    TaskQueue::TaskQueue(): m_inputTasksQueue{},
                            m_outputTasksQueue{},
                            m_currentId{0} {
        m_thread = std::thread{&TaskQueue::threadWork, this};
        m_thread.detach();
    }

    TaskQueue::~TaskQueue() {
        if(m_thread.joinable())
            m_thread.join();
    }

    void TaskQueue::threadWork() {
        bool m_running = true;
        while (m_running) {
            using namespace std::chrono_literals;

            std::unique_lock<std::mutex> lock(m_outputMutex);
            data_cond.wait(lock, [this]{ return !m_inputTasksQueue.empty(); });

            auto task = std::move(m_inputTasksQueue.front());
            if(!task)
                continue;

            m_inputTasksQueue.pop();
            task -> execute();

            std::this_thread::sleep_for(5000ms);
            m_outputTasksQueue.push(std::move(task));
        }
    }

    void TaskQueue::process() {
        if(!m_outputTasksQueue.empty()) {
            auto task = std::move(m_outputTasksQueue.front());
            m_outputTasksQueue.pop();
            task -> call();
        }
    }
}