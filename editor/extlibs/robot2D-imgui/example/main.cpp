#include <robot2D/Engine.hpp>
#include <robot2D/imgui/Gui.hpp>

namespace {
    // ImGui ShowDemo
    bool demoWindowOpen = false;
}

class MyApplication: public robot2D::Application {
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
private:
    void guiRender();
private:
    robot2D::Gui m_gui;
};

MyApplication::MyApplication(): m_gui{} {}

void MyApplication::setup() {
    m_gui.setup(*m_window);
}

void MyApplication::handleEvents(const robot2D::Event& event) {
    m_gui.handleEvents(event);
}

void MyApplication::guiUpdate(float deltaTime) {
    m_gui.update(deltaTime);
    guiRender();
}

void MyApplication::guiRender() {

    /// see Sugar.hpp for more on_## methods
//    on_Window("") {
//        onButton("ShowDemoWindow") {
//            demoWindowOpen = !demoWindowOpen;
//        }
//    }


    ImGui::ShowDemoWindow(&demoWindowOpen);
}

void MyApplication::render() {
    m_window -> clear();
    m_gui.render();
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