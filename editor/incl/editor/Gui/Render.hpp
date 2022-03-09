/*********************************************************************
(c) Alex Raag 2021
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

namespace ImGui {
    class GuiRender {
    public:
        GuiRender();
        ~GuiRender();

        void setup();
        void render(ImDrawData* drawData);
    private:
        bool setupGL();
        void setupFonts();
        void setupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, unsigned int );
    private:
        robot2D::Texture m_fontTexture;
        robot2D::ShaderHandler m_shader;
        unsigned VAO, VBO, EBO;
        OrthoView orthoView;
    };
}
