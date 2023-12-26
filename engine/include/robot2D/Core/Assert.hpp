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
#include <filesystem>
#include <robot2D/OptionConfig.hpp>

namespace robot2D {


    #ifdef RB_DEBUG
    #define RB_ASSERTS
    #define RB_DEBUGBREAK()
    #else
    #define RB_DEBUGBREAK()
    #endif

    #define RB_EXPAND_MACRO(x) x
    #define RB_STRINGIFY_MACRO(x) #x

    #ifdef RB_ASSERTS

    #define RB_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { RB##type##ERROR(msg, __VA_ARGS__); RB_DEBUGBREAK(); } }
    #define RB_INTERNAL_ASSERT_WITH_MSG(type, check, ...) RB_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
    #define RB_INTERNAL_ASSERT_NO_MSG(type, check) RB_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", RB_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

    #define RB_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
    #define RB_INTERNAL_ASSERT_GET_MACRO(...) RB_EXPAND_MACRO( RB_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_NO_MSG) )

    #define RB_ASSERT(...) RB_EXPAND_MACRO( RB_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
    #define RB_CORE_ASSERT(...) RB_EXPAND_MACRO( RB_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
    #else
        #define RB_ASSERT(...)
        #define RB_CORE_ASSERT(...)
    #endif

}