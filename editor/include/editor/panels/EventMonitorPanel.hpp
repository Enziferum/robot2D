#pragma once
#include "IPanel.hpp"
#include <editor/EventHistory.hpp>
#include <robot2D/imgui/WindowOptions.hpp>
#include <imgui/imgui.h>

namespace editor {
    /// A bounded raw-event trace. GUI delivery means submitted, not yet consumed.
    class EventMonitorPanel final : public IPanel {
    public:
        explicit EventMonitorPanel(EventHistory& history) :
            IPanel(UniqueType(typeid(EventMonitorPanel))), m_history(history) {
            setState(PanelState::Closed);
        }
        void render() override {
            robot2D::WindowOptions options;
            options.name = "Events###EventsPanel";
            renderWindow(options, [this]() {
                bool recording = m_history.recording();
                if(ImGui::Checkbox("Record", &recording)) m_history.setRecording(recording);
                ImGui::SameLine();
                if(ImGui::Button("Clear")) m_history.clear();
                ImGui::Text("Overwritten: %llu", static_cast<unsigned long long>(m_history.overwrittenCount()));
                ImGui::Text("Oversized skipped: %llu", static_cast<unsigned long long>(m_history.oversizedCount()));
                ImGui::InputInt("Type filter (-1 = all)", &m_filter);
                ImGui::TextUnformatted("GUI receives raw input; route shows the additional recipient.");
                if(ImGui::BeginTable("EventTrace", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Sequence"); ImGui::TableSetupColumn("Frame");
                    ImGui::TableSetupColumn("Received (us)"); ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Route");
                    ImGui::TableHeadersRow();
                    for(const auto& record : m_history.events()) {
                        if(m_filter >= 0 && static_cast<int>(record.event.type()) != m_filter) continue;
                        const auto& context = record.event.context();
                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("%llu", static_cast<unsigned long long>(context.sequence));
                        ImGui::TableNextColumn();
                        ImGui::Text("%llu", static_cast<unsigned long long>(context.frame)); ImGui::TableNextColumn();
                        ImGui::Text("%lld", static_cast<long long>(context.receivedAt.count()));
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(typeName(record.event.type()));
                        if(ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            if(const auto* key = record.event.getIf<robot2D::Event::KeyboardEvent>())
                                ImGui::Text("Key %d; scan %d; mods %u", static_cast<int>(key -> code), key -> scancode,
                                key -> modifiers);
                            else if(const auto* mouse = record.event.getIf<robot2D::Event::MouseButtonEvent>())
                                ImGui::Text("Button %d at %.1f, %.1f", mouse -> btn, mouse -> x, mouse -> y);
                            else if(const auto* move = record.event.getIf<robot2D::Event::MouseMoved>())
                                ImGui::Text("%.1f, %.1f", move -> x, move -> y);
                            else if(const auto* drop = record.event.getIf<robot2D::Event::FilesDropped>())
                                for(const auto& path : drop -> paths) ImGui::TextUnformatted(path.c_str());
                            ImGui::EndTooltip();
                        }
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(routeName(record.decision.target));
                        if(record.decision.cancelInputs) {
                            ImGui::SameLine(); ImGui::TextUnformatted("(cancel)");
                        }
                    }
                    ImGui::EndTable();
                }
            });
        }
    private:
        static const char* typeName(robot2D::Event::Type type) {
            static constexpr const char* names[] = {"None", "Resize", "Close request", "Focus gained", "Focus lost",
                                                    "Mouse move", "Scroll", "Mouse press", "Mouse release", "Key press",
                                                    "Key release", "Text", "Drop", "Joystick +", "Joystick -"};
            return names[static_cast<std::size_t>(type)];
        }
        static const char* routeName(InputRoute route) {
            switch(route) {
            case InputRoute::Gui: return "GUI";
            case InputRoute::Editor: return "Editor";
            case InputRoute::Runtime: return "Runtime";
            case InputRoute::System: return "System";
            default: return "Ignored";
            }
        }
        EventHistory& m_history;
        int m_filter{-1};
    };
}
