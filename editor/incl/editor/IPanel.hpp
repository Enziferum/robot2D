//
// Created by Necromant on 03.10.2021.
//

#pragma once
#include <memory>
#include "Defines.hpp"

namespace editor {
    class IPanel {
    public:
        using Ptr = std::shared_ptr<IPanel>;
    public:
        IPanel(UniqueType uniqueType);
        virtual ~IPanel() = 0;

        virtual void update(float dt);
        virtual void render() = 0;
        UniqueType getID() const { return m_id; }
    protected:
        UniqueType m_id;
    };
}
