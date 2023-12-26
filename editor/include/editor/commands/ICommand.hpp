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

#include <cstdint>
#include <memory>
#include <editor/ClassID.hpp>

namespace editor {
    using CommandID = std::uint32_t;


#define DECLARE_CLASS_ID(name) \
    virtual const class_id& who() const noexcept override \
    { \
        return id(); \
    } \
    static const class_id& id() noexcept;

#define IMPLEMENT_CLASS_ID(name) \
    const class_id&  ##name::id() noexcept { \
         static const class_id id{#name};   \
         return id;\
    }

    class ICommand {
    public:
        using Ptr = std::shared_ptr<ICommand>;
    public:
        virtual ~ICommand() = 0;
        virtual const class_id& who() const noexcept = 0;

        virtual void undo() = 0;
        virtual void redo();
    };

} // namespace viewer