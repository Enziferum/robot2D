/*********************************************************************
   (c) Alex Raag 2023
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

#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <robot2D/Config.hpp>
#include "Keyboard.hpp"

namespace robot2D {
    using EventTime = std::chrono::microseconds;

    struct EventContext {
        std::uint64_t sequence{0};
        std::uint64_t frame{0};
        EventTime receivedAt{0};
    };

    /** @brief An owning, immutable payload and its reception metadata.
     * Default construction yields None, suitable as a poll output argument.
     * Concrete alternatives are distinct types. Common payload bases can also
     * be queried safely (e.g. KeyboardEvent matches press and release).
     * receivedAt is callback reception time, not the physical OS input time.
     */
    class ROBOT2D_EXPORT_API Event {
    public:
        enum class Type {
            None, Resized, Closed, GainFocus, LostFocus, MouseMoved, MouseWheel,
            MousePressed, MouseReleased, KeyPressed, KeyReleased, TextEntered,
            DragDrop, JoystickConnected, JoystickDisconnected
        };

        struct SizeEvent { unsigned width{0}; unsigned height{0}; };
        struct KeyboardEvent { Key code{Key::Unknown}; int scancode{0}; unsigned modifiers{0}; };
        struct MouseButtonEvent { int btn{0}; float x{0}; float y{0}; unsigned modifiers{0}; };
        struct MouseMoveEvent { float x{0}; float y{0}; };
        struct MouseWheelEvent { float scroll_x{0}; float scroll_y{0}; float x{0}; float y{0}; };
        struct TextEnterEvent { unsigned symbol{0}; };
        struct JoystickEvent { int joystickId{0}; bool isGamepad{false}; };

        struct Resized : SizeEvent {};
        struct Closed {};
        struct GainFocus {};
        struct LostFocus {};
        struct MouseMoved : MouseMoveEvent {};
        struct MouseWheel : MouseWheelEvent {};
        struct MousePressed : MouseButtonEvent {};
        struct MouseReleased : MouseButtonEvent {};
        struct KeyPressed : KeyboardEvent { bool repeat{false}; };
        struct KeyReleased : KeyboardEvent {};
        struct TextEntered : TextEnterEvent {};
        struct FilesDropped { std::vector<std::string> paths; };
        struct JoystickConnected : JoystickEvent {};
        struct JoystickDisconnected : JoystickEvent {};

        using Data = std::variant<std::monostate, Resized, Closed, GainFocus, LostFocus,
                MouseMoved, MouseWheel, MousePressed, MouseReleased, KeyPressed,
                KeyReleased, TextEntered, FilesDropped, JoystickConnected, JoystickDisconnected>;

        Event() = default;
        template<typename Payload, typename = std::enable_if_t<
                !std::is_same_v<std::decay_t<Payload>, Event> && std::is_constructible_v<Data, Payload> > >
        explicit Event(Payload payload, EventContext context = {}) :
            m_data(std::move(payload)), m_context(context) {
        }

        Type type() const noexcept {
            return m_data.valueless_by_exception() ? Type::None : static_cast<Type>(m_data.index());
        }
        const EventContext& context() const noexcept {
            return m_context;
        }
        Event withFrame(std::uint64_t frame) && noexcept {
            m_context.frame = frame;
            return std::move(*this);
        }

        template<typename Payload>
        const Payload* getIf() const noexcept {
            if(m_data.valueless_by_exception())
                return nullptr;
            return std::visit([](const auto& value) -> const Payload* {
                using Value = std::decay_t<decltype(value)>;
                if constexpr(std::is_same_v<Payload, Value> || std::is_base_of_v<Payload, Value>)
                    return &value;
                else
                    return nullptr;
            }, m_data);
        }
        template<typename Payload>
        const Payload& get() const {
            if(const auto* value = getIf<Payload>())
                return *value;
            throw std::bad_variant_access();
        }
        template<typename Visitor>
        decltype(auto) visit(Visitor&& visitor) const {
            return std::visit(std::forward<Visitor>(visitor), m_data);
        }
    private:
        Data m_data;
        EventContext m_context;
    };

    struct FrameEvents {
        std::uint64_t frame{0};
        std::vector<Event> events;
    };
}
