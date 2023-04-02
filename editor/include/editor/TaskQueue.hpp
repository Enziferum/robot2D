#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include "Task.hpp"

namespace editor {
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

                auto function = std::make_shared<TaskFunction<T, Callback>>(std::forward<Callback>(callback));
                if(!function) {
                    //TODO: add logging
                    return -1;
                }
                auto task = std::make_shared<T>(function, std::forward<Args>(args)...);


                task -> m_taskID = m_currentId;
                ++m_currentId;
                if(!task) {
                    //TODO: add logging
                    return -1;
                }
            {
                std::lock_guard<std::mutex> lock(m_inputMutex);
                m_inputTasksQueue.push(task);
                data_cond.notify_one();
            }
            return 0;
        }

        void process();

        /// remove all ?
        void clear();
    private:
        TaskQueue();
        void threadWork();
    private:
        std::thread m_thread;
        std::mutex m_inputMutex;
        std::mutex m_outputMutex;
        std::queue<ITask::Ptr> m_inputTasksQueue;
        std::queue<ITask::Ptr> m_outputTasksQueue;
        std::condition_variable data_cond;
        std::uint32_t m_currentId;
        std::atomic_bool m_running;
    };
}