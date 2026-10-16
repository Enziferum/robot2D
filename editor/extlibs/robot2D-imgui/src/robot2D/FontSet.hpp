#pragma once
#include <robot2D/imgui/Gui.hpp>
#include <unordered_map>

namespace robot2D::priv {
    class FontSet {
    public:
        bool build(const GuiConfiguration& configuration, std::string& error);
        ImFont* get(FontId id) const noexcept;
        void clear();
    private:
        std::vector<GuiFontConfig> m_sources;
        std::unordered_map<FontId, ImFont*> m_fonts;
    };
}
