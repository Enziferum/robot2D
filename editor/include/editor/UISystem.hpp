#pragma once

#include <functional>
#include <robot2D/Ecs/System.hpp>
#include <robot2D/Core/Event.hpp>
#include "EditorCamera.hpp"

namespace editor {

    using MousePressedCallback = std::function<void(robot2D::ecs::Entity, std::uint64_t)>;
    using MouseHoverCallback = std::function<void(robot2D::ecs::Entity)>;

    class UISystem: public robot2D::ecs::System {
    public:
        enum Flags
        {
            RightMouse = 0x1,
            LeftMouse = 0x2,
            MiddleMouse = 0x4,
        };


        UISystem(robot2D::MessageBus&);
        ~UISystem() override = default;

        void handleEvents(const robot2D::Event& event);
        void update(float dt) override;

        void setCamera(IEditorCamera::Ptr camera) { m_editorCamera = camera; }

        std::uint32_t addMousePressedCallback(MousePressedCallback&& callback);
        std::uint32_t addMouseHoverCallback(MouseHoverCallback&& callback);
        std::uint32_t addMouseUnHoverCallback(MouseHoverCallback&& callback);

    private:
        std::vector<MousePressedCallback> m_mousePressedCallbacks;
        std::vector<MouseHoverCallback> m_mouseHoverCallbacks;
        std::vector<MouseHoverCallback> m_mouseUnHoverCallbacks;

        std::vector<Flags> m_mouseDownEvents;
        std::vector<Flags> m_mouseUpEvents;
        robot2D::vec2i m_eventPosition{};
        IEditorCamera::Ptr m_editorCamera{nullptr};
    };

} // namespace editor