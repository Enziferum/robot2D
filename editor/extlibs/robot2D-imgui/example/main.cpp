#include <robot2D/Engine.hpp>
#include <stdexcept>
#include <robot2D/imgui/Gui.hpp>
#include <robot2D/imgui/Sugar.hpp>

namespace {
    // ImGui ShowDemo
    bool showDemoWindow = false;
    bool demoWindowOpen = false;
}

class MyApplication : public robot2D::Application {
public:
    MyApplication();
    MyApplication(const MyApplication& other) = delete;
    MyApplication& operator=(const MyApplication& other) = delete;
    MyApplication(MyApplication&& other) = delete;
    MyApplication& operator=(MyApplication&& other) = delete;
    ~MyApplication() override = default;

    void setup() override;

    void handleEvents(const robot2D::Event& event) override;
    /// Make ImGui Update / Rendering only in guiUpdate method.
    void guiUpdate(float deltaTime) override;
    void render() override;
    void destroy() override {
        m_gui.shutdown();
    }
private:
    void guiRender();
private:
    robot2D::Gui m_gui;
};

MyApplication::MyApplication() : m_gui{} {
}

void MyApplication::setup() {
    robot2D::GuiConfiguration configuration;
    configuration.window = m_window;
    configuration.allowDefaultFallback = true;
    if(!m_gui.initialize(configuration)) throw std::runtime_error(m_gui.lastError());
}

void MyApplication::handleEvents(const robot2D::Event& event) {
    m_gui.handleEvents(event);
}

void MyApplication::guiUpdate(float deltaTime) {
    if(!m_gui.beginFrame(deltaTime)) throw std::runtime_error(m_gui.lastError());
    guiRender();
}

void MyApplication::guiRender() {

    /// see Sugar.hpp for more imgui_## methods
    imgui_Window("Sugar Example") {
        imgui_Button("ShowDemoWindow") {
            showDemoWindow = !showDemoWindow;
        }
    }

    if(showDemoWindow)
        ImGui::ShowDemoWindow(&demoWindowOpen);
}

void MyApplication::render() {
    m_window -> clear();
    m_gui.endFrame();
    m_window -> afterRender();
    if(!m_gui.render()) throw std::runtime_error(m_gui.lastError());
    m_window -> display();
}

#define SIMPLE_MAIN 0
#if SIMPLE_MAIN == 1
ROBOT2D_MAIN(MyApplication)
#elif SIMPLE_MAIN == 0
int main() {
    robot2D::EngineConfiguration engineConfiguration{
        {1280, 920},
        {"Robot2D + ImGui"}
    };
    ROBOT2D_RUN_ENGINE(MyApplication, engineConfiguration)
}
#endif