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

#include <robot2D/Graphics/GL.hpp>
#include "Render.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace robot2D {
    const std::string vertexShader =
        R"(
        #version 330 core
        layout(location = 0) in vec2 Position;
        layout(location = 1) in vec2 TexCoord;
        layout(location = 2) in vec4 Color;

        uniform mat4 ProjMtx;
        out vec2 FragUV;
        out vec4 FragColor;

        void main() {
            FragUV = TexCoord;
            FragColor = Color;
            gl_Position = ProjMtx * vec4(Position.xy, 0, 1.0);
        }
    )";

    const std::string fragmentShader =
        R"(
        #version 330 core
        in vec4 FragColor;
        in vec2 FragUV;

        uniform sampler2D Texture;
        layout (location = 0) out vec4 Out_Color;

        void main() {
            Out_Color = FragColor * texture(Texture, FragUV.st);
        }
    )";

    namespace {
        // Temporary GL boundary until Core exposes the agreed Graphics pass.
        // Current Core has no bind-eliding GL state cache; every draw rebinds its resources.
        struct GLState {
            GLint program, vao, arrayBuffer, activeTexture, texture, sampler, activeBinding, activeSampler;
            GLint drawFramebuffer, readFramebuffer, polygon[2], viewport[4], scissor[4];
            GLint srcRGB, dstRGB, srcAlpha, dstAlpha, eqRGB, eqAlpha;
            GLint unpackBuffer, unpackAlignment, unpackRowLength, unpackSkipPixels, unpackSkipRows;
            GLboolean colorMask[4], depthMask;
            static constexpr std::array<GLenum, 7> features = {GL_BLEND, GL_CULL_FACE, GL_DEPTH_TEST,
                                                               GL_STENCIL_TEST, GL_SCISSOR_TEST, GL_PRIMITIVE_RESTART,
                                                               GL_RASTERIZER_DISCARD};
            std::array<GLboolean, features.size()> enabled;
            GLState() {
                glGetIntegerv(GL_CURRENT_PROGRAM, &program);
                glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
                glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
                glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeBinding);
                glGetIntegerv(GL_SAMPLER_BINDING, &activeSampler);
                glActiveTexture(GL_TEXTURE0);
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
                glGetIntegerv(GL_SAMPLER_BINDING, &sampler);
                glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
                glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
                glGetIntegerv(GL_POLYGON_MODE, polygon);
                glGetIntegerv(GL_VIEWPORT, viewport);
                glGetIntegerv(GL_SCISSOR_BOX, scissor);
                glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB); glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
                glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
                glGetIntegerv(GL_BLEND_EQUATION_RGB, &eqRGB); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &eqAlpha);
                glGetBooleanv(GL_COLOR_WRITEMASK, colorMask); glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
                glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
                glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
                glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackRowLength);
                glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &unpackSkipPixels);
                glGetIntegerv(GL_UNPACK_SKIP_ROWS, &unpackSkipRows);
                for(std::size_t i = 0; i < features.size(); ++i) enabled[i] = glIsEnabled(features[i]);
            }
            ~GLState() {
                glUseProgram(program);
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture); glBindSampler(0, sampler);
                glActiveTexture(activeTexture);
                glBindTexture(GL_TEXTURE_2D, activeBinding);
                glBindSampler(activeTexture - GL_TEXTURE0, activeSampler);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
                glPolygonMode(GL_FRONT_AND_BACK, polygon[0]);
                glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
                glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
                glBlendEquationSeparate(eqRGB, eqAlpha);
                glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
                glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
                glDepthMask(depthMask);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
                glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, unpackRowLength);
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, unpackSkipPixels);
                glPixelStorei(GL_UNPACK_SKIP_ROWS, unpackSkipRows);
                for(std::size_t i = 0; i < features.size(); ++i) {
                    if(enabled[i]) glEnable(features[i]); else glDisable(features[i]);
                }
            }
        };
        struct VertexArrayOwner {
            GLuint id = 0;
            VertexArrayOwner() {
                glGenVertexArrays(1, &id);
            }
            ~VertexArrayOwner() {
                if(id) glDeleteVertexArrays(1, &id);
            }
        };
    }

    bool calculateGuiClip(ImVec4 clip, ImVec2 origin, ImVec2 scale, vec2i size, std::array<int, 4>& result) {
        if(size.x <= 0 || size.y <= 0 || !std::isfinite(scale.x) || !std::isfinite(scale.y)
            || scale.x <= 0 || scale.y <= 0) return false;
        const double x0 = (static_cast<double>(clip.x) - origin.x) * scale.x;
        const double y0 = (static_cast<double>(clip.y) - origin.y) * scale.y;
        const double x1 = (static_cast<double>(clip.z) - origin.x) * scale.x;
        const double y1 = (static_cast<double>(clip.w) - origin.y) * scale.y;
        if(!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(x1) || !std::isfinite(y1)
            || x1 <= x0 || y1 <= y0) return false;
        const auto clamp = [](double value, int maximum) {
            return std::clamp(value, 0.0, static_cast<double>(maximum));
        };
        const int left = static_cast<int>(std::floor(clamp(x0, size.x)));
        const int top = static_cast<int>(std::floor(clamp(y0, size.y)));
        const int right = static_cast<int>(std::ceil(clamp(x1, size.x)));
        const int bottom = static_cast<int>(std::ceil(clamp(y1, size.y)));
        result = {left, top, right - left, bottom - top};
        return right > left && bottom > top;
    }

    GuiRender::~GuiRender() {
        shutdown();
    }

    bool GuiRender::initialize() {
        if(m_ready) {
            m_error = "renderer already initialized"; return false;
        }
        m_error.clear();
        GLState previous;
        glGenBuffers(1, &m_vbo); glGenBuffers(1, &m_ebo);
        if(!m_vbo || !m_ebo
            || !m_shader.createShader(ShaderType::Vertex, vertexShader, false, "ImGui.vertex")
            || !m_shader.createShader(ShaderType::Fragment, fragmentShader, false, "ImGui.fragment")) {
            m_error = "GUI GPU setup failed: " + m_shader.getLastError(); shutdown(); return false;
        }
        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        ImGui::GetIO().Fonts -> GetTexDataAsRGBA32(&pixels, &width, &height);
        if(!pixels || width <= 0 || height <= 0) {
            m_error = "empty font atlas"; shutdown(); return false;
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0); glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        m_fontTexture.create({static_cast<unsigned>(width), static_cast<unsigned>(height)}, pixels, 1);
        GLint uploadedWidth = 0, uploadedHeight = 0;
        if(m_fontTexture.getID()) {
            glBindTexture(GL_TEXTURE_2D, m_fontTexture.getID());
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &uploadedWidth);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &uploadedHeight);
        }
        if(!m_fontTexture.getID() || uploadedWidth != width || uploadedHeight != height) {
            m_error = "font atlas GPU upload failed"; shutdown(); return false;
        }
        ImGui::GetIO().Fonts -> SetTexID(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(m_fontTexture.getID())));
        m_ready = true;
        return true;
    }

    void GuiRender::shutdown() noexcept {
        if(m_vbo) glDeleteBuffers(1, &m_vbo);
        if(m_ebo) glDeleteBuffers(1, &m_ebo);
        m_vbo = m_ebo = 0;
        m_fontTexture.release();
        m_shader.release();
        m_ready = false;
    }

    void GuiRender::setupRenderState(const ImDrawData& data, vec2i size, unsigned vao, unsigned target) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
        glEnable(GL_BLEND); glEnable(GL_SCISSOR_TEST);
        glDisable(GL_CULL_FACE); glDisable(GL_DEPTH_TEST); glDisable(GL_STENCIL_TEST);
        glDisable(GL_PRIMITIVE_RESTART); glDisable(GL_RASTERIZER_DISCARD);
        glDepthMask(GL_FALSE); glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, size.x, size.y);
        glActiveTexture(GL_TEXTURE0); glBindSampler(0, 0);
        m_view.update({data.DisplayPos.x, data.DisplayPos.y, data.DisplaySize.x, data.DisplaySize.y});
        m_shader.use(); m_shader.set("Texture", 0); m_shader.setMatrix("ProjMtx", m_view.getMatrix());
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glEnableVertexAttribArray(0); glEnableVertexAttribArray(1); glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
            reinterpret_cast<void*>(offsetof(ImDrawVert, pos)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
            reinterpret_cast<void*>(offsetof(ImDrawVert, uv)));
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert),
            reinterpret_cast<void*>(offsetof(ImDrawVert, col)));
    }

    bool GuiRender::render(const ImDrawData& data) {
        m_error.clear();
        if(!m_ready) {
            m_error = "renderer is not initialized"; return false;
        }
        const double width = static_cast<double>(data.DisplaySize.x) * data.FramebufferScale.x;
        const double height = static_cast<double>(data.DisplaySize.y) * data.FramebufferScale.y;
        if(!std::isfinite(width) || !std::isfinite(height) || width > std::numeric_limits<int>::max()
            || height > std::numeric_limits<int>::max()) {
            m_error = "invalid framebuffer size"; return false;
        }
        if(width <= 0 || height <= 0) return true;
        const vec2i size{static_cast<int>(width), static_cast<int>(height)};
        GLState previous;
        VertexArrayOwner vao;
        if(!vao.id) {
            m_error = "GUI VAO creation failed"; return false;
        }
        setupRenderState(data, size, vao.id, previous.drawFramebuffer);
        for(int n = 0; n < data.CmdListsCount; ++n) {
            const auto* list = data.CmdLists[n];
            glBufferData(GL_ARRAY_BUFFER, list -> VtxBuffer.Size * sizeof(ImDrawVert), list -> VtxBuffer.Data,
                GL_STREAM_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, list -> IdxBuffer.Size * sizeof(ImDrawIdx), list -> IdxBuffer.Data,
                GL_STREAM_DRAW);
            for(const auto& command : list -> CmdBuffer) {
                if(command.UserCallback) {
                    if(command.UserCallback != ImDrawCallback_ResetRenderState) command.UserCallback(list, &command);
                    // Restore all required UI bindings after foreign GL; no Core binding cache exists yet.
                    setupRenderState(data, size, vao.id, previous.drawFramebuffer);
                    continue;
                }
                if(command.VtxOffset != 0 || command.IdxOffset > static_cast<unsigned>(list -> IdxBuffer.Size)
                    || command.ElemCount > static_cast<unsigned>(list -> IdxBuffer.Size) - command.IdxOffset
                    || command.ElemCount > static_cast<unsigned>(std::numeric_limits<GLsizei>::max())) {
                    m_error = "unsupported or invalid GUI draw range"; return false;
                }
                std::array<int, 4> clip;
                if(!calculateGuiClip(command.ClipRect, data.DisplayPos, data.FramebufferScale, size, clip)) continue;
                glScissor(clip[0], size.y - clip[1] - clip[3], clip[2], clip[3]);
                glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(command.GetTexID())));
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.ElemCount),
                    sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                    reinterpret_cast<void*>(static_cast<intptr_t>(command.IdxOffset * sizeof(ImDrawIdx))));
            }
        }
        return true;
    }
}
