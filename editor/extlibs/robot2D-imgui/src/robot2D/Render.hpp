/*********************************************************************
   (c) Alex Raag 2024
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

#pragma once
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <imgui/imgui.h>
#include "OrthoView.hpp"
#include <array>
#include <string>

namespace robot2D {
    /// Top-left pixel rectangle, conservatively rounded outward and clamped to target.
    bool calculateGuiClip(ImVec4 clip, ImVec2 origin, ImVec2 scale, vec2i size, std::array<int, 4>& result);

    class GuiRender {
    public:
        GuiRender() = default;
        GuiRender(const GuiRender&) = delete;
        GuiRender& operator=(const GuiRender&) = delete;
        ~GuiRender();
        bool initialize();
        void shutdown() noexcept;
        bool render(const ImDrawData& drawData);
        const std::string& lastError() const noexcept {
            return m_error;
        }
    private:
        void setupRenderState(const ImDrawData& data, vec2i size, unsigned vao, unsigned target);
        Texture m_fontTexture;
        ShaderHandler m_shader;
        unsigned m_vbo = 0;
        unsigned m_ebo = 0;
        bool m_ready = false;
        std::string m_error;
        OrthoView m_view;
    };
}
