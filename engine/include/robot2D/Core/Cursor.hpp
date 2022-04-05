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

#pragma once

#include <string>
#include <memory>
#include <robot2D/Config.hpp>
#include "Vector2.hpp"

namespace robot2D {

    namespace priv {
        class CursorImpl;
    }

    using CursorHandle = void* ;

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
        /// Only initialize private implementation. To create use one of create methods.
        Cursor();
        ~Cursor();

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
        std::unique_ptr<priv::CursorImpl> m_cursorImpl;
    };
}
