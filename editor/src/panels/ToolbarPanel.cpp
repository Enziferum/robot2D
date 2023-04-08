#include <editor/panels/GameViewport.hpp>
#include <robot2D/imgui/Gui.hpp>
#include <robot2D/imgui/Api.hpp>

namespace editor {

    GameViewport::GameViewport(robot2D::MessageBus& messageBus):
    IPanel(UniqueType(typeid(GameViewport))),
    m_messageBus{messageBus}
    {}

    void GameViewport::render() {
        const auto& colors = ::ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];

        robot2D::WindowOptions windowOptions {
                {
                        {ImGuiStyleVar_WindowPadding, {0, 2}},
                        {ImGuiStyleVar_ItemInnerSpacing, {}}
                },
                {
                        {ImGuiCol_Button, robot2D::Color::fromGL(0.F, 0.F, 0.F, 0.F)},
                        {ImGuiCol_ButtonHovered, robot2D::Color::fromGL(buttonHovered.x,buttonHovered.z, 0.5F)},
                        {ImGuiCol_ButtonActive,robot2D::Color::fromGL(buttonActive.x,buttonActive.z, 0.5F)}
                }
        };
        windowOptions.name = "##toolbar";
        windowOptions.flagsMask = ImGuiWindowFlags_NoScrollbar;

        robot2D::createWindow(windowOptions, [this]() {});
    }
} // namespace editor