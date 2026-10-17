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

#include <string>
#include <memory>
#include <robot2D/Config.hpp>
#include "Vector2.hpp"

namespace robot2D {

    namespace priv {
        class CursorImpl;
    }

    class DesktopRuntime;

    using CursorHandle = void*;

    enum class CursorType {
        Arrow,
        TextInput,
        ResizeUpDown,
        ResizeLeftRight,
        Hand
    };

    /**
     * \brief Allow to use Cursor as object and set custom.
     */
    class ROBOT2D_EXPORT_API Cursor {
    public:
        /// Empty cursor for restoring the OS default. Native creation requires the runtime constructor.
        Cursor();
        explicit Cursor(DesktopRuntime& desktop);
        Cursor(const Cursor& other) = delete;
        Cursor& operator=(const Cursor& other) = delete;
        Cursor(Cursor&& other) = delete;
        Cursor& operator=(Cursor&& other) = delete;
        ~Cursor() noexcept;

        /**
         * \brief Allow to create own cursor from binary data.
         * \details Don't use Graphics module format such Image / Texture due to use Core module with Graphics Depedency.
         */
        bool createFromPixels(unsigned char* pixes, const vec2u& size);

        /// Default one is OS - specific Arrow style cursor.
        void createDefault();

        /// Allow to create specific cursor.
        void create(const CursorType& cursorType);

        /**
         * Returns OS specific or Internal Window Library.
         * Ex for Desktop Cursor is GlfwCursor.
         */
        CursorHandle getRaw() const;
    private:
        DesktopRuntime* m_desktop = nullptr;
        std::unique_ptr<priv::CursorImpl> m_cursorImpl;
    };
}
