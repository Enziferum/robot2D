//
// Created by Necromant on 05.10.2021.
//

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