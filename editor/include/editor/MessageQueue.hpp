#pragma once

#include <queue>
#include <vector>

namespace editor {
    template<typename T>
    struct Message {

    };

    class MessageQueue {
    public:
        MessageQueue();
        ~MessageQueue() = default;

        bool postMessage();
    private:
    };
}