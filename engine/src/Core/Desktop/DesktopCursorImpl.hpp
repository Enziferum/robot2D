#pragma once

#include <GLFW/glfw3.h>
#include "../CursorImpl.hpp"

namespace robot2D {
    namespace priv {
        class DesktopCursorImpl: public CursorImpl {
        public:
            DesktopCursorImpl();
            ~DesktopCursorImpl() override = default;

            
        private:
        };
    }
}