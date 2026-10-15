#include <editor/panels/ConsolePanel.hpp>
#include <robot2D/imgui/Api.hpp>
#include <algorithm>
#include <utility>
#include <vector>
#include "../IconsFontsAwesome5.hpp"

namespace editor {

    namespace {
        const char* levelNames[] = {"Trace", "Debug", "Info", "Warning", "Error", "Critical", "Other"};
    }

    ConsolePanel::ConsolePanel(std::string path):
        IPanel{UniqueType(typeid(ConsolePanel))}, m_log{std::move(path)} {
        m_log.poll();
    }

    void ConsolePanel::update(float deltaTime) {
        m_pollTime += deltaTime;
        if(m_pollTime >= 0.1f) {
            m_pollTime = 0.f;
            m_log.poll();
        }
    }

    void ConsolePanel::render() {
        robot2D::WindowOptions options;
        options.flagsMask = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        options.name = ICON_FA_TERMINAL " Console###ConsolePanel";
        renderWindow(options, [this]() {
            if(ImGui::BeginTabBar("##ConsoleSources")) {
                if(ImGui::BeginTabItem("Engine")) {
                    renderEngineLog();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        });
    }

    void ConsolePanel::renderEngineLog() {
        if(ImGui::Button("Clear"))
            m_log.clear();
        ImGui::SameLine();
        const bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        if(ImGui::Button("Levels"))
            ImGui::OpenPopup("##LogLevels");
        if(ImGui::BeginPopup("##LogLevels")) {
            for(std::size_t index = 0; index < m_levels.size(); ++index)
                ImGui::Checkbox(levelNames[index], &m_levels[index]);
            ImGui::EndPopup();
        }
        if(ImGui::GetContentRegionAvail().x >= 160.f)
            ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_autoScroll);
        m_filter.Draw("Search", std::max(40.f, ImGui::GetContentRegionAvail().x - 65.f));

        const auto& entries = m_log.getEntries();
        std::vector<std::size_t> visible;
        std::string clipboard;
        for(std::size_t index = 0; index < entries.size(); ++index) {
            const auto& entry = entries[index];
            if(m_levels[static_cast<std::size_t>(entry.level)] && m_filter.PassFilter(entry.text.c_str())) {
                visible.push_back(index);
                if(copy)
                    clipboard += entry.text + '\n';
            }
        }
        if(copy)
            ImGui::SetClipboardText(clipboard.c_str());
        if(!m_log.getStatus().empty())
            ImGui::TextDisabled("%s", m_log.getStatus().c_str());
        const auto source = std::string{"Source: "} + m_log.getPath();
        ImGui::TextDisabled("%s", source.c_str());
        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", source.c_str());
        if(ImGui::BeginChild("##LogLines", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
            const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.f;
            if(visible.empty())
                ImGui::TextDisabled(entries.empty() ? "No log entries" : "No matching entries");
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(visible.size()));
            while(clipper.Step()) {
                for(int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    const auto& entry = entries[visible[row]];
                    auto color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
                    if(entry.level == LogLevel::Warning)
                        color = {0.95f, 0.75f, 0.40f, 1.f};
                    else if(entry.level == LogLevel::Error || entry.level == LogLevel::Critical)
                        color = {0.97f, 0.48f, 0.48f, 1.f};
                    else if(entry.level == LogLevel::Trace || entry.level == LogLevel::Debug)
                        color = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
                    ImGui::PushID(row);
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextUnformatted(entry.text.c_str());
                    ImGui::PopStyleColor();
                    if(ImGui::BeginPopupContextItem("##LogLine")) {
                        if(ImGui::MenuItem("Copy line"))
                            ImGui::SetClipboardText(entry.text.c_str());
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            }
            if(m_autoScroll && atBottom && m_renderedVersion != m_log.getVersion())
                ImGui::SetScrollY(ImGui::GetCursorPosY());
            m_renderedVersion = m_log.getVersion();
        }
        ImGui::EndChild();
    }

}
