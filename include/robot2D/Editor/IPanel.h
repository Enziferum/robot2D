//
// Created by support on 26.03.2021.
//

#pragma once

#include <memory>
#include <robot2D/Core/Event.h>

namespace robot2D{
    class IPanel{
    public:
        using Ptr = std::shared_ptr<IPanel>;
    public:
        IPanel();
        virtual ~IPanel() = 0;

        virtual void handleEvents(const Event& event) = 0;
        virtual void update(float dt) = 0;
    };
}
