#include <editor/runtime/PlayerApplication.hpp>
#include <editor/runtime/SceneAssets.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/components/ButtonComponent.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/scripting/ScriptingGlue.hpp>
#include <robot2D/Util/Logger.hpp>
#include <cmath>

namespace editor::runtime {
    namespace {
        class PlayerCamera final : public IEditorCamera {
        public:
            explicit PlayerCamera(robot2D::RenderWindow* window) : IEditorCamera(EditorCameraType::Orthographic),
                m_window(window) {
            }
            void setViewportSize(robot2D::vec2f) override {
            }
            void handleEvents(const robot2D::Event&) override {
            }
            void update(robot2D::vec2f, float) override {
            }
            robot2D::vec2f convertPixelToCoords(robot2D::vec2f mouse) const override {
                return m_window -> mapPixelToCoords(mouse.as<int>());
            }
        private:
            robot2D::RenderWindow* m_window;
        };
        class PlayerBridge final : public ScriptInteractor {
        public:
            PlayerBridge(Scene::Ptr scene, ScriptEngine* engine, std::function<void()> quit) :
                ScriptInteractor(EditorInteractor::WeakPtr{}, engine), m_scene(std::move(scene)),
                m_quit(std::move(quit)) {
            }
            SceneEntity getEntity(std::uint64_t id) override {
                return m_scene -> getEntity(UUID{id});
            }
            SceneEntity getEntity(const std::string& name) override {
                return m_scene -> getEntity(name);
            }
            void exitEngineRuntime() override {
                m_quit();
            }
            bool loadSceneRuntime(std::string&& name) override {
                RB_EDITOR_ERROR(
                    "[Player] Scene transitions are unavailable; requested='{}'; requires RUNTIME-01/SCENE-01", name);
                return false;
            }
            void loadSceneAsyncRuntime(std::string&& name) override {
                loadSceneRuntime(std::move(name));
            }
            SceneEntity duplicateRuntime(SceneEntity, robot2D::vec2f) override {
                RB_EDITOR_ERROR("[Player] Instantiate is unavailable; requires the RUNTIME-01 lifecycle contract");
                return {};
            }
        private:
            Scene::Ptr m_scene;
            std::function<void()> m_quit;
        };
    }
    PlayerApplication::PlayerApplication(LaunchConfig config, std::shared_ptr<PlayerResult> result) :
        m_config(std::move(config)), m_result(std::move(result)) {
    }

    void PlayerApplication::setup() {
        try {
            RB_EDITOR_INFO("[Player] Load started; package='{}'; scene='{}'", m_config.root.u8string(),
                m_config.scene.u8string());
            auto coreBase = m_config.core; coreBase.replace_extension();
            m_engine.Init(coreBase.u8string(), m_config.monoLib.u8string(), m_config.monoEtc.u8string());
            m_monoInitialized = true;
            if(!m_engine.runtimeInit(m_config.scripts.u8string())) throw std::runtime_error(
                          "Cannot load game script assembly");
            m_engine.SetWindow(m_window);
            m_scene = std::make_shared<Scene>(m_bus);
            m_scene -> setAssociatedProjectPath(m_config.root.u8string());
            m_scripts = std::make_shared<PlayerBridge>(m_scene, &m_engine, [this]() { m_running = false; });
            m_engine.setScriptInteractor(m_scripts);
            SceneSerializer reader(m_scene);
            if(!reader.deserialize(m_config.scene.u8string(),
                m_scripts)) throw std::runtime_error(reader.getErrorMessage());
            unsigned cameras = 0;
            m_scene -> forEachEntity([&](SceneEntity& entity) {
                if(entity.hasComponent<CameraComponent>() && entity.getComponent<CameraComponent>().isPrimary) {
                    const auto& camera = entity.getComponent<CameraComponent>();
                    if(!entity.hasComponent<DrawableComponent>() || !std::isfinite(camera.size.x) ||
                    !std::isfinite(camera.size.y) || camera.size.x <= 0 || camera.size.y <= 0)
                        throw std::runtime_error("Primary camera requires Drawable and a positive finite size");
                    ++cameras;
                    m_window -> setViewVirtual(m_window -> getSize(), robot2D::View(robot2D::FloatRect{
                        camera.position.x, camera.position.y, camera.size.x, camera.size.y}));
                }
                if(entity.hasComponent<ScriptComponent>()) {
                    const auto& name = entity.getComponent<ScriptComponent>().name;
                    if(!name.empty() &&
                    !m_engine.hasEntityClass(name)) throw std::runtime_error("Missing script class: " + name);
                }
            });
            if(cameras != 1) throw std::runtime_error("Player requires exactly one primary scene camera");
            loadSceneAssets(m_scene);
            m_scene -> forEachEntity([&](SceneEntity& entity) { activateEntityAssets(m_scene, entity, true); });
            m_scene -> forEachEntity([&](SceneEntity& entity) {
                if(entity.hasComponent<ButtonComponent>()) {
                    m_scene -> registerUICallback(entity);
                    entity.getComponent<ButtonComponent>().onClickCallback =
                    [weak = std::weak_ptr<ScriptInteractor>(m_scripts)](UUID id, const std::string& method) {
                        if(auto scripts = weak.lock()) scripts -> callScriptMethod(id, method);
                    };
                }
            });
            m_camera = std::make_shared<PlayerCamera>(m_window);
            m_scene -> setEditorCamera(m_camera);
            m_engine.SetCamera(m_camera);
            m_scene -> onRuntimeStart(m_scripts);
            m_scene -> setRuntimeCamera(true);
            m_scene -> setRuntimeWindowSize(m_window -> getSize());
            RB_EDITOR_INFO("[Player] Runtime started; scene='{}'", m_config.scene.u8string());
        }
        catch(const std::exception& error) { fail(error.what()); }
    }
    void PlayerApplication::fail(const std::string& reason) {
        RB_EDITOR_ERROR("[Player] Failed; scene='{}'; reason='{}'", m_config.scene.u8string(), reason);
        m_result -> exitCode = 1; m_running = false;
    }
    void PlayerApplication::destroy() {
        if(m_scene && m_scene -> isRunning()) m_scene -> onRuntimeStop(m_scripts);
        m_engine.setScriptInteractor(nullptr);
        m_scripts.reset(); m_scene.reset(); m_camera.reset();
        if(m_monoInitialized) {
            m_engine.Shutdown(); m_monoInitialized = false;
        }
        ScriptGlue::setService(nullptr);
        LocalResourceManager::getManager() -> releaseGraphicsResources();
        RB_EDITOR_INFO("[Player] Runtime stopped; exit_code={}", m_result -> exitCode);
    }
    void PlayerApplication::handleEvents(const robot2D::Event& event) {
        if(event.type == robot2D::Event::Closed) m_running = false;
        if(event.type == robot2D::Event::Resized && m_scene && event.size.widht && event.size.heigth)
            m_scene -> setRuntimeWindowSize({event.size.widht, event.size.heigth});
        if(m_scene && m_running) m_scene -> handleEventsRuntime(event);
    }
    void PlayerApplication::update(float dt) {
        if(!m_running || !m_scene) return;
        try { m_scene -> updateRuntime(dt, m_scripts); m_hasTick = true; }
        catch(const std::exception& error) { fail(error.what()); }
    }
    void PlayerApplication::render() {
        if(!m_running || !m_scene || !m_hasTick) return;
        try {
            m_window -> beforeRender(); m_window -> clear(robot2D::Color::Black);
            m_window -> draw(*m_scene); m_window -> afterRender();
        }
        catch(const std::exception& error) { fail(error.what()); }
    }
}
