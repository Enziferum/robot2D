#pragma once
#include <memory>

namespace editor {
    class ITaskFunction {
    public:
        using Ptr = std::shared_ptr<ITaskFunction>;
        virtual ~ITaskFunction() = 0;

        virtual void execute(void* buffer) = 0;
    };

    template<typename T, typename Func>
    class TaskFunction: public ITaskFunction {
    public:
        TaskFunction(Func&& func): m_func{std::forward<Func>(func)} {}
        ~TaskFunction() override = default;

        void execute(void* buffer) override {
            if(!buffer) {
                // TODO log
                return;
            }
            const T* funcData = static_cast<T*>(buffer);
            m_func(*funcData);
        }
    private:
        Func m_func;
    };
}