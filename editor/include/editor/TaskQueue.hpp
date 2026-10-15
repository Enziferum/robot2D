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
#include <numeric>
#include <limits>
#include <type_traits>
#include <utility>

#include "Task.hpp"
#include <robot2D/Util/Logger.hpp>

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
        Done,
        Failed,
        Cancelled
    };

    using TaskID = std::uint32_t;
    constexpr std::uint32_t InvalidTaskId = std::numeric_limits<std::uint32_t>::max();

    struct TaskInfo {
        TaskStatus status;
        std::exception_ptr exceptionPtr { nullptr };
    };

    class TaskQueue {
    public:
        TaskQueue();
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
            return addAsyncTaskWithCompletion<T>(
                [success = CallbackStorage(std::forward<Callback>(callback))](const T& task) mutable {
                    if(task.getOutcome() == TaskOutcome::Succeeded)
                        success(task);
                }, std::forward<Args>(args)...);
        }

        // Receives success, failure or task-observed cancellation on the process() thread.
        // requestStop() prevents new dispatches; stop() also synchronizes with an in-flight callback.
        template<typename T, typename ...Args, typename Callback>
        TaskID addAsyncTaskWithCompletion(Callback&& callback, Args&& ...args) {
            {
                std::lock_guard lock(m_mutex);
                if(m_stopping)
                    return InvalidTaskId;
            }
            using CallbackStorage = std::decay_t<Callback>;
            auto function = std::make_shared<TaskFunction<T, CallbackStorage>>(
                    CallbackStorage(std::forward<Callback>(callback)));
            auto task = std::make_shared<T>(function, std::forward<Args>(args)...);
            TaskID taskID;
            {
                std::lock_guard lock(m_mutex);
                if(m_stopping || m_taskId == InvalidTaskId)
                    return InvalidTaskId;
                taskID = m_taskId++;
                task -> m_taskID = taskID;
                m_statuses.emplace(taskID, TaskInfo{TaskStatus::Started});
                m_inputTasksQueue.push(std::move(task));
            }
            data_cond.notify_one();
            return taskID;
        }

        void process();
        void requestStop();
        void stop();
        bool hasPendingTasks() const;
        TaskStatus getStatus(TaskID taskId);
        TaskInfo getTaskInfo(TaskID taskId) const;
        // Discards waiting tasks and undelivered results; the active task can still complete.
        void clear();
    private:
        void threadWork();
        void discard(std::queue<ITask::Ptr>& tasks);
    private:
        std::thread m_thread;
        std::thread::id m_workerID;
        mutable std::mutex m_mutex;
        std::mutex m_requestMutex;
        std::mutex m_stopMutex;
        std::recursive_mutex m_callbackMutex;
        std::queue<ITask::Ptr> m_inputTasksQueue;
        std::queue<ITask::Ptr> m_outputTasksQueue;
        ITask::Ptr m_activeTask;
        std::unordered_map<TaskID, TaskInfo> m_statuses;
        std::condition_variable data_cond;
        TaskID m_taskId { 0 };
        bool m_stopping { false };
        std::size_t m_activeCallbacks { 0 };
    };
}
