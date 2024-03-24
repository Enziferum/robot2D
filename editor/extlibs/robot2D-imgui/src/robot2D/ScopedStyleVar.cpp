#include <robot2D/imgui/ScopedStyleVar.hpp>

namespace robot2D {

    template<>
    ScopedStyleVar<float>::ScopedStyleVar(ImGuiStyleVar ImGuiVar, const float& value): m_count{ 1 } {
        ImGui::PushStyleVar(ImGuiVar, value);
    }

    template<>
    ScopedStyleVar<float>::ScopedStyleVar(std::initializer_list<ImGuiPair> values) {
        for(auto value: values)
            ImGui::PushStyleVar(value.first, value.second);
        m_count = static_cast<int>(values.size());
    }

    template<>
    ScopedStyleVar<float>::~ScopedStyleVar(){
        ImGui::PopStyleVar(m_count);
    }


    template<>
    ScopedStyleVar<ImVec2>::ScopedStyleVar(ImGuiStyleVar ImGuiVar, const ImVec2& value): m_count{ 1 } {
        ImGui::PushStyleVar(ImGuiVar, value);
    }


    template<>
    ScopedStyleVar<ImVec2>::ScopedStyleVar(std::initializer_list<ImGuiPair> values) {
        for(auto value: values)
            ImGui::PushStyleVar(value.first, value.second);
        m_count = static_cast<int>(values.size());
    }

    template<>
    ScopedStyleVar<ImVec2>::~ScopedStyleVar(){
        ImGui::PopStyleVar(m_count);
    }
}