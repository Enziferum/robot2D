#pragma once

#include <unordered_map>
#include <memory>

#include <robot2D/Core/Message.hpp>
#include "Macro.hpp"

namespace editor {


    class IFunction {
    public:
        using Ptr = std::unique_ptr<IFunction>;
        virtual ~IFunction() = 0;
        virtual void execute(const robot2D::Message& message) = 0;
    };

    template<typename Callback, typename CallbackFuncArg>
    class FunctionWrapper: public IFunction{
    public:
        FunctionWrapper(Callback&& callback): m_callback{callback} {}
        ~FunctionWrapper() override = default;

        void execute(const robot2D::Message& message) override {
            auto& data = message.getData<CallbackFuncArg>();
            //std::forward<CallbackFuncArg>(data)
            m_callback(data);
        }
    private:
        Callback m_callback;
    };

    template<typename Callback>
    class FunctionWrapper<Callback, void>: public IFunction {
    public:
        FunctionWrapper(Callback&& callback): m_callback{callback} {}
        ~FunctionWrapper() override = default;

        void execute(const robot2D::Message& message) override {
            m_callback();
        }
    private:
        Callback m_callback;
    };

    class MessageDispatcher {
    public:
        MessageDispatcher();
        MessageDispatcher(const MessageDispatcher&)=delete;
        MessageDispatcher& operator=(const MessageDispatcher&)=delete;
        MessageDispatcher(MessageDispatcher&&)=delete;
        MessageDispatcher& operator=(MessageDispatcher&&)=delete;
        ~MessageDispatcher() = default;

        template<typename CallbackFuncArg, typename Callback>
        void onMessage(robot2D::Message::ID messageId, Callback&& callback) {
            auto ptr = std::make_unique<FunctionWrapper<Callback, CallbackFuncArg>>(std::forward<Callback>(callback));
            if(!ptr)
                return;
            m_functions.template emplace(std::pair<robot2D::Message::ID,
                            IFunction::Ptr>(messageId, std::move(ptr)));
        }

        void process(const robot2D::Message& message);
    private:
        std::unordered_map<robot2D::Message::ID, IFunction::Ptr> m_functions;
    };
}