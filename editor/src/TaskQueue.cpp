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
            data_cond.wait(lock, [this] {
                return !m_inputTasksQueue.empty()
                       || !m_running.load(std::memory_order::memory_order_relaxed);
            });

            if (m_inputTasksQueue.empty())
                continue;
            auto task = m_inputTasksQueue.front();
            if (!task)
                continue;

            m_inputTasksQueue.pop();

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

    void TaskQueue::stop() {
        m_running.store(false, std::memory_order::memory_order_relaxed);
        {
            std::unique_lock<std::mutex> lock(m_inputMutex);
            data_cond.notify_all();
        }

        if(m_thread.joinable())
            m_thread.join();

    }
}