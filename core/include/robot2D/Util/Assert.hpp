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

#include <cstdio>
#include <cstdlib>
#include <utility>
#include <robot2D/Util/Logger.hpp>

namespace robot2D::detail {
    [[noreturn]] inline void terminateAssertion(bool core, const char* expression,
        const char* file, int line, const char* message) noexcept {
        const auto* separator = message[0] == '\0' ? "" : ": ";
        std::fprintf(stderr, "Assertion '%s' failed at %s:%d%s%s\n", expression, file, line, separator, message);
        std::fflush(stderr);
        try {
            const auto& log = core ? logger::Log::getCoreLogger() : logger::Log::getClientLogger();
            log -> critical("Assertion '{}' failed at {}:{}{}{}", expression, file, line, separator, message);
        }
        catch(const std::exception& error) {
            std::fprintf(stderr, "Assertion logging failed: %s\n", error.what());
        }
        catch(...) {
            std::fprintf(stderr, "Assertion logging failed: unknown error\n");
        }
        std::fflush(stderr);
        std::abort();
    }

    [[noreturn]] inline void assertionFailed(bool core, const char* expression,
        const char* file, int line) noexcept {
        terminateAssertion(core, expression, file, line, "");
    }

    template<typename ... Args>
    [[noreturn]] void assertionFailed(bool core, const char* expression, const char* file, int line,
        spdlog::format_string_t<Args...> format, Args&&... args) noexcept {
        try {
            const auto message = fmt::format(format, std::forward<Args>(args)...);
            terminateAssertion(core, expression, file, line, message.c_str());
        }
        catch(...) {
            terminateAssertion(core, expression, file, line, "Failed to format assertion message");
        }
    }
}

// RB_ASSERTS is a presence-based opt-in for Release, retained for compatibility.
#if !defined(NDEBUG) && !defined(RB_ASSERTS)
    #define RB_ASSERTS
#endif

#define RB_EXPAND_MACRO(x) x
#define RB_STRINGIFY_MACRO(x) #x

/// @brief Explicitly terminates with std::abort in every build configuration.
#define RB_DEBUGBREAK() std::abort()

#ifdef RB_ASSERTS
    #define RB_INTERNAL_ASSERT_NO_MSG(core, \
                check) do { if(!(check)) { ::robot2D::detail::assertionFailed(core, #check, __FILE__, __LINE__); \
                            } } while(false)
    #define RB_INTERNAL_ASSERT_WITH_MSG(core, check, \
                ...) do { if(!(check)) { ::robot2D::detail::assertionFailed(core, #check, __FILE__, __LINE__, \
                                             __VA_ARGS__); } } while(false)
    #define RB_INTERNAL_ASSERT_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, NAME, \
                ...) NAME
    #define RB_INTERNAL_ASSERT_SELECT(...) RB_EXPAND_MACRO(RB_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__, \
                RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, \
                RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, \
                RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, \
                RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, \
                RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, RB_INTERNAL_ASSERT_WITH_MSG, \
                RB_INTERNAL_ASSERT_NO_MSG, unused))

/// @brief Checks once and aborts on failure; optional fmt message arguments are evaluated only on failure.
    #define RB_ASSERT(...) RB_EXPAND_MACRO(RB_INTERNAL_ASSERT_SELECT(__VA_ARGS__)(false, __VA_ARGS__))
/// @brief Like RB_ASSERT, using the core logging channel on failure.
    #define RB_CORE_ASSERT(...) RB_EXPAND_MACRO(RB_INTERNAL_ASSERT_SELECT(__VA_ARGS__)(true, __VA_ARGS__))
#else
/// @brief Disabled assertions evaluate neither the condition nor message arguments.
    #define RB_ASSERT(...) do { } while(false)
/// @brief Disabled core assertions evaluate neither the condition nor message arguments.
    #define RB_CORE_ASSERT(...) do { } while(false)
#endif
