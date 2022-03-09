/*********************************************************************
(c) Alex Raag 2021
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

#include "DesktopCursorImpl.hpp"
namespace robot2D {
    namespace priv {
        int convertCursorType(const CursorType& cursorType) {
            switch(cursorType) {
                case CursorType::Arrow:
                    return GLFW_ARROW_CURSOR;
                case CursorType::TextInput:
                    return GLFW_IBEAM_CURSOR;
                case CursorType::ResizeUpDown:
                    return GLFW_VRESIZE_CURSOR;
                case CursorType::ResizeLeftRight:
                    return GLFW_HRESIZE_CURSOR;
                case CursorType::Hand:
                    return GLFW_HAND_CURSOR;
            }
            return GLFW_ARROW_CURSOR;
        }

        DesktopCursorImpl::DesktopCursorImpl(): m_cursor(nullptr) {}

        DesktopCursorImpl::~DesktopCursorImpl() {
            if(m_cursor != nullptr)
                glfwDestroyCursor(m_cursor);
        }

        bool DesktopCursorImpl::createFromPixels(unsigned char* pixels, const vec2u& size) {
            GLFWimage image;
            image.width = size.x;
            image.height = size.y;
            image.pixels = pixels;
            if(m_cursor != nullptr)
                glfwDestroyCursor(m_cursor);
            m_cursor = glfwCreateCursor(&image, 0, 0);
            if(m_cursor == nullptr)
                return false;
            return true;
        }

        void DesktopCursorImpl::createDefault() {
            if(m_cursor != nullptr)
                glfwDestroyCursor(m_cursor);
            m_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        }

        void DesktopCursorImpl::create(const CursorType& cursorType) {
            if(m_cursor != nullptr)
                glfwDestroyCursor(m_cursor);
            m_cursor = glfwCreateStandardCursor(convertCursorType(cursorType));
        }

        void* DesktopCursorImpl::getRaw() const {
            return (void*)m_cursor;
        }
    }
}