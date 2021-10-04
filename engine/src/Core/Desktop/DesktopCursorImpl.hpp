#pragma once

#include <GLFW/glfw3.h>
#include "../CursorImpl.hpp"

namespace robot2D {
    namespace priv {
        class DesktopCursorImpl: public CursorImpl {
        public:
            DesktopCursorImpl();
            ~DesktopCursorImpl() override;

            bool createFromPixels(unsigned char* pixels, const vec2u &size) override;
            void createDefault() override;
            void create(const CursorType& cursorType) override;

            void* getRaw() const override;

        private:
            GLFWcursor* m_cursor;
        };
    }
}