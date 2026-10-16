#pragma once
#include <unordered_map>
#include <robot2D/Core/Event.hpp>
#include <robot2D/imgui/InputEvents.hpp>

namespace editor {
    enum class InputRoute { Ignored, Gui, Editor, Runtime, System };
    struct RoutingContext {
        robot2D::GuiInputCapture capture;
        bool mouseInViewport{false};
        bool viewportFocused{false};
        bool runtime{false};
    };
    struct RouteDecision {
        InputRoute target{InputRoute::Ignored};
        bool cancelInputs{false};
    };

    /** @brief Main-thread routing; does not suppress raw delivery to ImGui.
     * A key/button keeps its owner through release. The first held mouse button
     * owns the gesture, including subsequent buttons. Popup, focus loss and mode
     * changes cancel application gestures; unmatched releases never start actions.
     */
    class InputRouter {
    public:
        RouteDecision route(const robot2D::Event& event, const RoutingContext& context) {
            using Event = robot2D::Event;
            bool cancel = false;
            if(context.runtime != m_runtime || (context.capture.popup && !m_popup)) {
                reset();
                cancel = true;
            }
            m_runtime = context.runtime;
            m_popup = context.capture.popup;
            if(event.getIf<Event::LostFocus>()) {
                reset(); m_focused = false;
                return {InputRoute::System, true};
            }
            if(event.getIf<Event::GainFocus>()) {
                m_focused = true;
                return {InputRoute::System, cancel};
            }
            const auto scene = context.runtime ? InputRoute::Runtime : InputRoute::Editor;
            const auto mouseTarget = [&]() {
                if(!m_focused || context.capture.popup || context.capture.activeItem) return InputRoute::Gui;
                if(context.mouseInViewport) return scene;
                return context.capture.mouse ? InputRoute::Gui : InputRoute::Ignored;
            };
            if(const auto* key = event.getIf<Event::KeyPressed>()) {
                auto found = m_keys.find(key -> code);
                if(found != m_keys.end()) return {found -> second, cancel};
                if(key -> repeat) return {InputRoute::Ignored, cancel};
                const bool blocked = !m_focused || context.capture.popup || context.capture.activeItem ||
                    (context.capture.keyboard && !context.viewportFocused) ||
                    (context.runtime && !context.viewportFocused);
                const auto target = blocked ? InputRoute::Gui : scene;
                m_keys.emplace(key -> code, target);
                return {target, cancel};
            }
            if(const auto* key = event.getIf<Event::KeyReleased>()) {
                const auto found = m_keys.find(key -> code);
                if(found == m_keys.end()) return {InputRoute::Ignored, cancel};
                const auto target = found -> second;
                m_keys.erase(found);
                return {target, cancel};
            }
            if(const auto* button = event.getIf<Event::MousePressed>()) {
                const auto target = m_buttons.empty() ? mouseTarget() : m_mouseOwner;
                m_buttons[button -> btn] = target;
                m_mouseOwner = target;
                return {target, cancel};
            }
            if(const auto* button = event.getIf<Event::MouseReleased>()) {
                const auto found = m_buttons.find(button -> btn);
                if(found == m_buttons.end()) return {InputRoute::Ignored, cancel};
                const auto target = found -> second;
                m_buttons.erase(found);
                if(m_buttons.empty()) m_mouseOwner = InputRoute::Ignored;
                return {target, cancel};
            }
            if(event.getIf<Event::MouseMoved>())
                return {m_buttons.empty() ? mouseTarget() : m_mouseOwner, cancel};
            if(event.getIf<Event::MouseWheel>()) return {mouseTarget(), cancel};
            if(event.getIf<Event::TextEntered>()) return {InputRoute::Gui, cancel};
            return {InputRoute::System, cancel};
        }
        void reset() noexcept {
            m_keys.clear(); m_buttons.clear(); m_mouseOwner = InputRoute::Ignored;
        }
    private:
        std::unordered_map<robot2D::Key, InputRoute> m_keys;
        std::unordered_map<int, InputRoute> m_buttons;
        InputRoute m_mouseOwner{InputRoute::Ignored};
        bool m_focused{true};
        bool m_runtime{false};
        bool m_popup{false};
    };
}
