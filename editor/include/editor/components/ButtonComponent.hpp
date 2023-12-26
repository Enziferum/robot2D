#pragma once

#include <functional>
#include <editor/Uuid.hpp>

namespace editor {

    struct ButtonComponent {
        bool hasEntity() const {
            return scriptEntity != 0;
        }

        UUID scriptEntity{0};
        std::function<void(UUID, const std::string&)> onClickCallback{nullptr};
        std::string clickMethodName;
    };

} // namespace editor