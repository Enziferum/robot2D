#include <robot2D/imgui/InputEvents.hpp>

namespace {
    ImGuiKey toGuiKey(robot2D::Key key) {
        using robot2D::Key;
        switch(key) {
        case Key::SPACE: return ImGuiKey_Space;
        case Key::APOSTROPHE: return ImGuiKey_Apostrophe;
        case Key::COMMA: return ImGuiKey_Comma;
        case Key::MINUS: return ImGuiKey_Minus;
        case Key::PERIOD: return ImGuiKey_Period;
        case Key::SLASH: return ImGuiKey_Slash;
        case Key::SEMICOLON: return ImGuiKey_Semicolon;
        case Key::EQUAL: return ImGuiKey_Equal;
        case Key::LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case Key::RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case Key::BACKSLASH: return ImGuiKey_Backslash;
        case Key::GRAVE_ACCENT: return ImGuiKey_GraveAccent;
        case Key::ESCAPE: return ImGuiKey_Escape;
        case Key::ENTER: return ImGuiKey_Enter;
        case Key::TAB: return ImGuiKey_Tab;
        case Key::BACKSPACE: return ImGuiKey_Backspace;
        case Key::INSERT: return ImGuiKey_Insert;
        case Key::DEL: return ImGuiKey_Delete;
        case Key::RIGHT: return ImGuiKey_RightArrow;
        case Key::LEFT: return ImGuiKey_LeftArrow;
        case Key::DOWN: return ImGuiKey_DownArrow;
        case Key::UP: return ImGuiKey_UpArrow;
        case Key::PAGE_UP: return ImGuiKey_PageUp;
        case Key::PAGE_DOWN: return ImGuiKey_PageDown;
        case Key::HOME: return ImGuiKey_Home;
        case Key::END: return ImGuiKey_End;
        case Key::CAPS_LOCK: return ImGuiKey_CapsLock;
        case Key::SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case Key::NUM_LOCK: return ImGuiKey_NumLock;
        case Key::PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case Key::PAUSE: return ImGuiKey_Pause;
        case Key::KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        case Key::KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case Key::KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case Key::KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case Key::KP_ADD: return ImGuiKey_KeypadAdd;
        case Key::KP_ENTER: return ImGuiKey_KeypadEnter;
        case Key::KP_EQUAL: return ImGuiKey_KeypadEqual;
        case Key::LEFT_SHIFT: return ImGuiKey_LeftShift;
        case Key::LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case Key::LEFT_ALT: return ImGuiKey_LeftAlt;
        case Key::LEFT_SUPER: return ImGuiKey_LeftSuper;
        case Key::RIGHT_SHIFT: return ImGuiKey_RightShift;
        case Key::RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case Key::RIGHT_ALT: return ImGuiKey_RightAlt;
        case Key::RIGHT_SUPER: return ImGuiKey_RightSuper;
        case Key::MENU: return ImGuiKey_Menu;
        case Key::A: return ImGuiKey_A;
        case Key::B: return ImGuiKey_B;
        case Key::C: return ImGuiKey_C;
        case Key::D: return ImGuiKey_D;
        case Key::E: return ImGuiKey_E;
        case Key::F: return ImGuiKey_F;
        case Key::G: return ImGuiKey_G;
        case Key::H: return ImGuiKey_H;
        case Key::I: return ImGuiKey_I;
        case Key::J: return ImGuiKey_J;
        case Key::K: return ImGuiKey_K;
        case Key::L: return ImGuiKey_L;
        case Key::M: return ImGuiKey_M;
        case Key::N: return ImGuiKey_N;
        case Key::O: return ImGuiKey_O;
        case Key::P: return ImGuiKey_P;
        case Key::Q: return ImGuiKey_Q;
        case Key::R: return ImGuiKey_R;
        case Key::S: return ImGuiKey_S;
        case Key::T: return ImGuiKey_T;
        case Key::U: return ImGuiKey_U;
        case Key::V: return ImGuiKey_V;
        case Key::W: return ImGuiKey_W;
        case Key::X: return ImGuiKey_X;
        case Key::Y: return ImGuiKey_Y;
        case Key::Z: return ImGuiKey_Z;
        case Key::Num0: return ImGuiKey_0;
        case Key::Num1: return ImGuiKey_1;
        case Key::Num2: return ImGuiKey_2;
        case Key::Num3: return ImGuiKey_3;
        case Key::Num4: return ImGuiKey_4;
        case Key::Num5: return ImGuiKey_5;
        case Key::Num6: return ImGuiKey_6;
        case Key::Num7: return ImGuiKey_7;
        case Key::Num8: return ImGuiKey_8;
        case Key::Num9: return ImGuiKey_9;
        case Key::KP_0: return ImGuiKey_Keypad0;
        case Key::KP_1: return ImGuiKey_Keypad1;
        case Key::KP_2: return ImGuiKey_Keypad2;
        case Key::KP_3: return ImGuiKey_Keypad3;
        case Key::KP_4: return ImGuiKey_Keypad4;
        case Key::KP_5: return ImGuiKey_Keypad5;
        case Key::KP_6: return ImGuiKey_Keypad6;
        case Key::KP_7: return ImGuiKey_Keypad7;
        case Key::KP_8: return ImGuiKey_Keypad8;
        case Key::KP_9: return ImGuiKey_Keypad9;
        case Key::F1: return ImGuiKey_F1;
        case Key::F2: return ImGuiKey_F2;
        case Key::F3: return ImGuiKey_F3;
        case Key::F4: return ImGuiKey_F4;
        case Key::F5: return ImGuiKey_F5;
        case Key::F6: return ImGuiKey_F6;
        case Key::F7: return ImGuiKey_F7;
        case Key::F8: return ImGuiKey_F8;
        case Key::F9: return ImGuiKey_F9;
        case Key::F10: return ImGuiKey_F10;
        case Key::F11: return ImGuiKey_F11;
        case Key::F12: return ImGuiKey_F12;
        case Key::F13: return ImGuiKey_F13;
        case Key::F14: return ImGuiKey_F14;
        case Key::F15: return ImGuiKey_F15;
        case Key::F16: return ImGuiKey_F16;
        case Key::F17: return ImGuiKey_F17;
        case Key::F18: return ImGuiKey_F18;
        case Key::F19: return ImGuiKey_F19;
        case Key::F20: return ImGuiKey_F20;
        case Key::F21: return ImGuiKey_F21;
        case Key::F22: return ImGuiKey_F22;
        case Key::F23: return ImGuiKey_F23;
        case Key::F24: return ImGuiKey_F24;
        default: return ImGuiKey_None;
        }
    }

    void submitModifiers(ImGuiIO& io, unsigned modifiers) {
        io.AddKeyEvent(ImGuiMod_Shift, (modifiers & 1u) != 0);
        io.AddKeyEvent(ImGuiMod_Ctrl, (modifiers & 2u) != 0);
        io.AddKeyEvent(ImGuiMod_Alt, (modifiers & 4u) != 0);
        io.AddKeyEvent(ImGuiMod_Super, (modifiers & 8u) != 0);
    }
}

namespace robot2D {
    void feedGuiEvent(ImGuiIO& io, const Event& event) {
        if(const auto* key = event.getIf<Event::KeyboardEvent>()) {
            submitModifiers(io, key -> modifiers);
            const auto translated = toGuiKey(key -> code);
            if(translated != ImGuiKey_None) {
                const auto* pressed = event.getIf<Event::KeyPressed>();
                if(!pressed || !pressed -> repeat)
                    io.AddKeyEvent(translated, pressed != nullptr);
                io.SetKeyEventNativeData(translated, static_cast<int>(key -> code), key -> scancode);
            }
        }
        else if(const auto* button = event.getIf<Event::MouseButtonEvent>()) {
            submitModifiers(io, button -> modifiers);
            io.AddMousePosEvent(button -> x, button -> y);
            if(button -> btn >= 0 && button -> btn < IM_ARRAYSIZE(io.MouseDown))
                io.AddMouseButtonEvent(button -> btn, event.getIf<Event::MousePressed>() != nullptr);
        }
        else if(const auto* position = event.getIf<Event::MouseMoved>())
            io.AddMousePosEvent(position -> x, position -> y);
        else if(const auto* wheel = event.getIf<Event::MouseWheel>()) {
            io.AddMousePosEvent(wheel -> x, wheel -> y);
            io.AddMouseWheelEvent(wheel -> scroll_x, wheel -> scroll_y);
        }
        else if(const auto* text = event.getIf<Event::TextEntered>())
            io.AddInputCharacter(text -> symbol);
        else if(event.getIf<Event::LostFocus>()) io.AddFocusEvent(false);
        else if(event.getIf<Event::GainFocus>()) io.AddFocusEvent(true);
    }

    GuiInputCapture captureGuiInput() {
        if(!ImGui::GetCurrentContext()) return {};
        const auto& io = ImGui::GetIO();
        return {io.WantCaptureMouse, io.WantCaptureKeyboard, ImGui::IsAnyItemActive(),
                ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)};
    }
}
