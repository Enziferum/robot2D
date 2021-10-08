#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace robot2D {

    class Log
    {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };

}


// Core log macros
#define RB_CORE_TRACE(...)    ::robot2D::Log::getCoreLogger()->trace(__VA_ARGS__)
#define RB_CORE_INFO(...)     ::robot2D::Log::getCoreLogger()->info(__VA_ARGS__)
#define RB_CORE_WARN(...)     ::robot2D::Log::getCoreLogger()->warn(__VA_ARGS__)
#define RB_CORE_ERROR(...)    ::robot2D::Log::getCoreLogger()->error(__VA_ARGS__)
#define RB_CORE_CRITICAL(...) ::robot2D::Log::getCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define RB_TRACE(...)         ::robot2D::Log::getClientLogger()->trace(__VA_ARGS__)
#define RB_INFO(...)          ::robot2D::Log::getClientLogger()->info(__VA_ARGS__)
#define RB_WARN(...)          ::robot2D::Log::getClientLogger()->warn(__VA_ARGS__)
#define RB_ERROR(...)         ::robot2D::Log::getClientLogger()->error(__VA_ARGS__)
#define RB_CRITICAL(...)      ::robot2D::Log::getClientLogger()->critical(__VA_ARGS__)