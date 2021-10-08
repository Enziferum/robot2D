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

#include <memory>
#include <robot2D/Graphics/RenderWindow.hpp>
#include "sandbox/Sandbox.hpp"
#include "sandbox/Render2DScene.hpp"

#define ROBOT2D_SCENE
#ifdef ROBOT2D_SCENE

int main() {
    robot2D::RenderWindow window{{1280, 920}, {"Sandbox"}, {true, false}};

    Sandbox sandbox(window);

    //// Put own scenes here /////
    Render2DScene::Ptr render2DScene = std::make_unique<Render2DScene>(window);
    render2DScene -> setup();
    //// Put own scenes here /////

    sandbox.setScene(std::move(render2DScene));
    sandbox.run();

    return 0;
}
#else

namespace logger {
    /// RBLogger v0.1 Single Thread Solution
    enum class LogLevel {
        Info,
        Warn,
        Error
    };

    class RBLogger {
    public:
        RBLogger() = default;

        ~RBLogger() = default;

        template<typename ... Args>
        void warn(Args &&... args) {
            baseLog(LogLevel::Warn, std::forward<Args>(args)...);
        }

        template<typename ... Args>
        void error(Args &&... args) {
            baseLog(LogLevel::Error, std::forward<Args>(args)...);
        }

        template<typename ... Args>
        void info(Args &&... args) {
            baseLog(LogLevel::Info, std::forward<Args>(args)...);
        }

    private:
        template<typename ... Args>
        void baseLog(const LogLevel &logLevel, Args &&... args);

    private:

    };

    template<typename ... Args>
    void RBLogger::baseLog(const LogLevel &logLevel, Args &&... args) {

    }

    class Logger {
    public:
        static void Init();

        static std::shared_ptr<RBLogger>& getCoreLogger() { return coreLogger; }
        static std::shared_ptr<RBLogger>& getUserLogger() { return userLogger; }
    private:
        static std::shared_ptr<RBLogger> coreLogger;
        static std::shared_ptr<RBLogger> userLogger;
    };

    std::shared_ptr<RBLogger> Logger::coreLogger = nullptr;
    std::shared_ptr<RBLogger> Logger::userLogger = nullptr;

    void Logger::Init() {
        coreLogger = std::make_shared<RBLogger>();
        userLogger = std::make_shared<RBLogger>();
    }
}

#define RB_LOG_WARN(...) logger::Logger::getCoreLogger() -> warn(__VA_ARGS__);
#define RB_LOG_ERROR(...) logger::Logger::getCoreLogger() -> error(__VA_ARGS__);
#define RB_LOG_INFO(...) logger::Logger::getCoreLogger() -> info(__VA_ARGS__);


int main() {
    logger::Logger::Init();

    RB_LOG_INFO("Info Message")
    RB_LOG_ERROR("Error Message")
    RB_LOG_WARN("Warn Message")

    return 0;
}
#endif