/*********************************************************************
(c) Alex Raag 2024
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

#include <unordered_map>
#include <memory>

#include <robot2D/Core/Message.hpp>
#include "Macro.hpp"

namespace editor {

    class IFunction {
    public:
        using Ptr = std::unique_ptr<IFunction>;
        virtual ~IFunction() = 0;
        virtual void execute(robot2D::Message& message) = 0;
    };

    template<typename Callback, typename CallbackFuncArg>
    class FunctionWrapper: public IFunction{
    public:
        FunctionWrapper(Callback&& callback): m_callback{callback} {}
        ~FunctionWrapper() override = default;

        void execute(robot2D::Message& message) override {
            if (message.needUnpack()) {
                auto data = message.unpack<CallbackFuncArg>();
                m_callback(data);
            }
            else {
                auto& data = message.getData<CallbackFuncArg>();

                //std::forward<CallbackFuncArg>(data)
                m_callback(data);
            }
            
        }
    private:
        Callback m_callback;
    };

    template<typename Callback>
    class FunctionWrapper<Callback, void>: public IFunction {
    public:
        FunctionWrapper(Callback&& callback): m_callback{callback} {}
        ~FunctionWrapper() override = default;

        void execute(robot2D::Message& message) override {
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

        /// TODO(a.raag) check Callback signature CallbackFuncArg
        template<typename CallbackFuncArg, typename Callback, typename = std::enable_if_t<std::is_invocable_v<Callback, CallbackFuncArg>>>
        void onMessage(robot2D::Message::ID messageId, Callback&& callback) {
            auto ptr = std::make_unique<FunctionWrapper<Callback, CallbackFuncArg>>(std::forward<Callback>(callback));
            if(!ptr)
                return;
            m_functions.emplace(std::pair<robot2D::Message::ID,
                            IFunction::Ptr>(messageId, std::move(ptr)));
        }

        void process(robot2D::Message& message);
    private:
        std::unordered_multimap<robot2D::Message::ID, IFunction::Ptr> m_functions;
    };

} // namespace editor