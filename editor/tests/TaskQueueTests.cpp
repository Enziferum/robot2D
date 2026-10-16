#include <editor/TaskQueue.hpp>
#include <editor/async/ExportTask.hpp>
#include "TestSupport.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <stdexcept>

namespace {
    using namespace std::chrono_literals;

    void require(bool value, const char* message) {
        if(!value)
            throw std::runtime_error(message);
    }

    template<typename Predicate>
    bool waitFor(Predicate predicate) {
        const auto deadline = std::chrono::steady_clock::now() + 5s;
        while(!predicate()) {
            if(std::chrono::steady_clock::now() >= deadline)
                return false;
            std::this_thread::sleep_for(1ms);
        }
        return true;
    }

    class ProbeTask;
    struct Probe {
        std::function<void(ProbeTask&)> work;
        std::atomic_int executed { 0 };
        std::atomic_int destroyed { 0 };
        std::atomic_bool cancellationRequested { false };
        std::atomic<editor::TaskID> actualID { editor::InvalidTaskId };
    };

    class ProbeTask : public editor::ITask {
    public:
        ProbeTask(editor::ITaskFunction::Ptr callback, std::shared_ptr<Probe> probe) :
            ITask(std::move(callback)), m_probe(std::move(probe)) {
        }
        ~ProbeTask() override {
            ++m_probe -> destroyed;
        }
        void execute() override {
            ++m_probe -> executed;
            m_probe -> actualID = getTaskID();
            if(m_probe -> work)
                m_probe -> work(*this);
        }
        void requestCancellation() noexcept override {
            m_probe -> cancellationRequested = true;
        }
        void failNow() {
            fail("Explicit failure");
        }
        void cancelNow() {
            markCancelled();
        }
    private:
        std::shared_ptr<Probe> m_probe;
    };

    void empty() {
        editor::TaskQueue queue;
        require(!queue.hasPendingTasks(), "Empty queue reports pending work");
        queue.stop();
        queue.stop();
        auto probe = std::make_shared<Probe>();
        require(queue.addAsyncTask<ProbeTask>([](const auto&) {}, probe) == editor::InvalidTaskId,
            "Stopped queue accepted new work");
        require(probe -> executed == 0 && probe -> destroyed == 0,
            "Stopped queue constructed or executed a task");
    }

    void completion() {
        editor::TaskQueue queue;
        int successCalls = 0;
        int failureCalls = 0;
        int cancelledCalls = 0;
        auto success = std::make_shared<Probe>();
        const auto successID = queue.addAsyncTask<ProbeTask>([&](const auto& task) {
            require(task.getOutcome() == editor::TaskOutcome::Succeeded, "Wrong success result");
            ++successCalls;
        }, success);
        auto throwing = std::make_shared<Probe>();
        throwing -> work = [](ProbeTask&) { throw std::runtime_error("Expected worker failure"); };
        const auto throwingID = queue.addAsyncTaskWithCompletion<ProbeTask>([&](const auto& task) {
            require(task.getOutcome() == editor::TaskOutcome::Failed && task.getException(),
            "Exception did not reach completion");
            ++failureCalls;
        }, throwing);
        auto failed = std::make_shared<Probe>();
        failed -> work = [](ProbeTask& task) { task.failNow(); };
        const auto failedID = queue.addAsyncTask<ProbeTask>([&](const auto&) { ++successCalls; }, failed);
        auto cancelled = std::make_shared<Probe>();
        cancelled -> work = [](ProbeTask& task) { task.cancelNow(); };
        const auto cancelledID = queue.addAsyncTaskWithCompletion<ProbeTask>([&](const auto& task) {
            require(task.getOutcome() == editor::TaskOutcome::Cancelled, "Wrong cancellation result");
            ++cancelledCalls;
        }, cancelled);
        require(waitFor([&]() { return queue.getStatus(cancelledID) == editor::TaskStatus::Cancelled; }),
            "Worker did not continue after failure");
        require(queue.hasPendingTasks(), "Ready results were not counted as pending");
        for(int i = 0; i < 4; ++i)
            queue.process();
        require(successCalls == 1 && failureCalls == 1 && cancelledCalls == 1,
            "Success/failure/cancellation callbacks were mixed");
        require(queue.getStatus(successID) == editor::TaskStatus::Done &&
            queue.getStatus(failedID) == editor::TaskStatus::Failed &&
            queue.getTaskInfo(throwingID).exceptionPtr, "Incorrect task statuses");
        require(success -> actualID == successID && throwing -> actualID == throwingID,
            "Task IDs were not assigned to tasks");
        require(!queue.hasPendingTasks(), "Completed callbacks left pending work");
    }

    void exportUnavailable() {
        tests::Directory files;
        const auto output = files / "existing output";
        tests::fs::create_directory(output); tests::write(output / "keep.txt", "author output");
        editor::TaskQueue queue;
        int completions = 0;
        const auto enqueue = [&](editor::ExportOptions options, const std::string& reason) {
            return queue.addAsyncTaskWithCompletion<editor::ExportTask>([&, reason](const auto& task) {
                require(task.getOutcome() == editor::TaskOutcome::Failed, "Unavailable exporter reported success");
                require(task.getErrorMessage().find(reason) != std::string::npos, "Build diagnostic was lost");
                ++completions;
            }, options); // options dies here: the queued task must own its copy.
        };
        enqueue({}, "directory");
        enqueue({false, output.u8string(), ""}, "scene");
        const auto id = enqueue({false, output.u8string(), "assets/scenes/main.scene"}, "PLAYER-01");
        require(waitFor([&]() {
            return queue.getStatus(id) == editor::TaskStatus::Failed;
        }), "Export task did not fail");
        for(int i = 0; i < 3; ++i) queue.process();
        require(completions == 3, "Build failure did not reach every completion");
        require(tests::read(output / "keep.txt") == "author output", "Export modified existing output");
        require(std::distance(tests::fs::directory_iterator(output), tests::fs::directory_iterator{}) == 1,
            "Unavailable export wrote files");
    }

    void shutdown() {
        editor::TaskQueue queue;
        std::promise<void> release;
        auto released = release.get_future().share();
        auto active = std::make_shared<Probe>();
        active -> work = [released](ProbeTask&) { released.wait(); };
        int callbacks = 0;
        const auto activeID = queue.addAsyncTask<ProbeTask>([&](const auto&) { ++callbacks; }, active);
        const bool started = waitFor([&]() { return active -> executed == 1; });
        if(!started) {
            release.set_value();
            require(false, "Active task did not start");
        }
        auto waiting = std::make_shared<Probe>();
        // This enqueue must return even though execute() is blocked.
        const auto waitingID = queue.addAsyncTask<ProbeTask>([&](const auto&) { ++callbacks; }, waiting);
        const bool pending = queue.hasPendingTasks();
        auto stopping = std::async(std::launch::async, [&]() { queue.stop(); });
        const bool signalled = waitFor([&]() { return active -> cancellationRequested.load(); });
        const bool waitingForActive = stopping.wait_for(0ms) != std::future_status::ready;
        const auto rejected = queue.addAsyncTask<ProbeTask>([](const auto&) {}, std::make_shared<Probe>());
        release.set_value();
        stopping.get();
        queue.process();
        require(pending && signalled && waitingForActive, "Shutdown did not signal/wait for active work");
        require(rejected == editor::InvalidTaskId && callbacks == 0, "Work/callback accepted during shutdown");
        require(waiting -> executed == 0 && waiting -> destroyed == 1 && active -> destroyed == 1,
            "Tasks were not released before stop returned");
        require(queue.getStatus(activeID) == editor::TaskStatus::Cancelled &&
            queue.getStatus(waitingID) == editor::TaskStatus::Cancelled && !queue.hasPendingTasks(),
            "Shutdown left pending work");
    }

    void clear() {
        editor::TaskQueue queue;
        int callbacks = 0;
        std::vector<std::shared_ptr<Probe> > probes;
        editor::TaskID lastID = editor::InvalidTaskId;
        for(int i = 0; i < 7; ++i) {
            auto probe = std::make_shared<Probe>();
            lastID = queue.addAsyncTask<ProbeTask>([&](const auto&) { ++callbacks; }, probe);
            probes.push_back(probe);
        }
        require(waitFor([&]() { return queue.getStatus(lastID) == editor::TaskStatus::Done; }),
            "Tasks did not finish");
        queue.clear();
        queue.process();
        require(callbacks == 0 && !queue.hasPendingTasks(), "clear left undelivered results");
        for(const auto& probe: probes)
            require(waitFor([&]() { return probe -> destroyed == 1; }), "clear did not release every result");
        auto next = std::make_shared<Probe>();
        lastID = queue.addAsyncTask<ProbeTask>([&](const auto&) { ++callbacks; }, next);
        require(waitFor([&]() { return queue.getStatus(lastID) == editor::TaskStatus::Done; }),
            "clear stopped the queue");
        // stop must also release ready results without dispatching them.
        queue.stop();
        require(next -> destroyed == 1 && callbacks == 0, "stop retained or delivered a ready result");
    }

    void callback() {
        editor::TaskQueue queue;
        auto first = std::make_shared<Probe>();
        const auto throwingID = queue.addAsyncTask<ProbeTask>([](const auto&) {
            throw std::runtime_error("Expected callback failure");
        }, first);
        require(waitFor([&]() { return queue.getStatus(throwingID) == editor::TaskStatus::Done; }),
            "Task did not finish");
        queue.process();
        require(queue.getStatus(throwingID) == editor::TaskStatus::Failed &&
            queue.getTaskInfo(throwingID).exceptionPtr, "Callback exception was lost");
        bool called = false;
        const auto next = queue.addAsyncTask<ProbeTask>([&](const auto&) {
            called = true;
            queue.stop();
        }, std::make_shared<Probe>());
        require(waitFor([&]() { return queue.getStatus(next) == editor::TaskStatus::Done; }),
            "Worker did not survive callback failure");
        queue.process();
        require(called && !queue.hasPendingTasks(), "stop from callback deadlocked or left pending work");
    }

    void dispatch() {
        editor::TaskQueue queue;
        std::promise<void> release;
        auto released = release.get_future().share();
        std::atomic_bool entered { false };
        std::atomic_bool stoppingEntered { false };
        std::atomic_bool exited { false };
        const auto id = queue.addAsyncTask<ProbeTask>([&](const auto&) {
            entered = true;
            released.wait();
            exited = true;
        }, std::make_shared<Probe>());
        require(waitFor([&]() { return queue.getStatus(id) == editor::TaskStatus::Done; }),
            "Task did not finish before dispatch");
        auto processing = std::async(std::launch::async, [&]() { queue.process(); });
        const bool callbackStarted = waitFor([&]() { return entered.load(); });
        auto stopping = std::async(std::launch::async, [&]() {
            stoppingEntered = true;
            queue.stop();
        });
        const bool stopStarted = waitFor([&]() { return stoppingEntered.load(); });
        const bool stopWaited = stopping.wait_for(10ms) != std::future_status::ready;
        release.set_value();
        processing.get();
        stopping.get();
        require(callbackStarted && stopStarted && stopWaited && exited,
            "stop did not synchronize with an in-flight completion callback");
        require(!queue.hasPendingTasks(), "Dispatch shutdown left pending work");
    }
}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        require(argc == 2, "Expected test case name");
        const std::string name = argv[1];
        if(name == "empty") empty();
        else if(name == "completion") {
            completion(); exportUnavailable();
        }
        else if(name == "shutdown") shutdown();
        else if(name == "clear") clear();
        else if(name == "callback") callback();
        else if(name == "dispatch") dispatch();
        else require(false, "Unknown test case");
        std::cout << name << " passed\n";
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
