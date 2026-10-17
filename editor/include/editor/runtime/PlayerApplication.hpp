#pragma once
#include <chrono>
#include <robot2D/Application.hpp>
#include <editor/runtime/LaunchConfig.hpp>
#include <editor/Scene.hpp>
#include <editor/scripting/ScriptingEngine.hpp>

namespace editor::runtime {
    struct PlayerResult { int exitCode{0}; };
    class PlayerApplication : public robot2D::Application {
    public:
        PlayerApplication(LaunchConfig config, std::shared_ptr<PlayerResult> result);
        void setup() override;
        void destroy() override;
        void handleEvents(const robot2D::Event& event) override;
        void update(float dt) override;
        void render() override;
    private:
        void fail(const std::string& reason);
        void startupStep(const char* phase);
        LaunchConfig m_config;
        std::shared_ptr<PlayerResult> m_result;
        robot2D::MessageBus m_bus;
        Scene::Ptr m_scene;
        ScriptEngine m_engine;
        std::shared_ptr<ScriptInteractor> m_scripts;
        IEditorCamera::Ptr m_camera;
        bool m_monoInitialized{false};
        bool m_hasTick{false};
        bool m_hasPresentedFrame{false};
        std::chrono::steady_clock::time_point m_startupBegin{std::chrono::steady_clock::now()};
        std::chrono::steady_clock::time_point m_startupStage{m_startupBegin};
    };
}
