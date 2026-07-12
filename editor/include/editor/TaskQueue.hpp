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

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <unordered_map>

#include "Task.hpp"

namespace editor {



    template<typename T>
    class SimpleThreadSafeQueue {
    public:
        SimpleThreadSafeQueue();
        SimpleThreadSafeQueue(const SimpleThreadSafeQueue& other) = delete;
        SimpleThreadSafeQueue& operator=(const SimpleThreadSafeQueue& other) = delete;
        SimpleThreadSafeQueue(SimpleThreadSafeQueue&& other) = delete;
        SimpleThreadSafeQueue& operator=(SimpleThreadSafeQueue&& other) = delete;
        ~SimpleThreadSafeQueue();


        void push(const T& value);
        void push(T&& value);


        template<typename ...Args>
        void emplate(Args&& ...args);

    private:
        std::mutex m_mutex;
        std::deque<T> m_queue;
    };


    enum class TaskStatus {
        Bad,
        Started,
        Processing,
        Done
    };

    using TaskID = std::uint32_t;
    // using InvalidTaskId = 0;

    struct TaskInfo {
        TaskStatus status;
        std::exception_ptr exceptionPtr { nullptr };
    };

    class TaskQueue {
    public:
        TaskQueue(const TaskQueue&) = delete;
        TaskQueue& operator=(const TaskQueue&) = delete;
        TaskQueue(TaskQueue&&) = delete;
        TaskQueue& operator=(TaskQueue&&) = delete;
        ~TaskQueue();

        static TaskQueue* GetQueue() {
            static TaskQueue taskQueue;
            return &taskQueue;
        }

        template<typename T, typename ... Args, typename Callback>
        TaskID addAsyncTask(Callback&& callback, Args&& ...args) {

            using CallbackStorage = std::decay_t<Callback>;

            auto function = std::make_shared<TaskFunction<T, CallbackStorage>>(
                    std::forward<Callback>(callback)
            );
            if(!function) {
                //TODO: add logging
                return -1;
            }
            auto task = std::make_shared<T>(function, std::forward<Args>(args)...);
            const auto taskID =
                    m_taskId.fetch_add(1, std::memory_order::memory_order_relaxed);

            {
                std::lock_guard<std::mutex> lock(m_inputMutex);
                m_inputTasksQueue.push(task);
            }

            data_cond.notify_one();

            return taskID;
        }

        void process();
        void stop();

        /// TODO(a.raag): create correct method
        bool hasPendingTasks() const { return false; }


        TaskStatus getStatus(TaskID taskId);

        /// remove all ?
        void clear();
    private:
        TaskQueue();

        void threadWork();

        void setStatus(TaskID taskId);
        void setException(TaskID taskId);
    private:
        std::thread m_thread;

        std::mutex m_inputMutex;
        std::mutex m_outputMutex;
        std::queue<ITask::Ptr> m_inputTasksQueue;
        std::queue<ITask::Ptr> m_outputTasksQueue;


        std::mutex m_statusMutex;
        std::unordered_map<TaskID, TaskInfo> m_statuses;

        std::condition_variable data_cond;
        std::atomic_int m_taskId;
        std::atomic_bool m_running;
    };
}