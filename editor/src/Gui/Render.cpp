#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Util/Logger.hpp>

#include <editor/Gui/Render.hpp>
#include <editor/Gui/Util.hpp>

namespace ImGui {
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    const GLchar* vertex_shader =
            "#version 330 core\n"
            "layout (location = 0) in vec2 Position;\n"
            "layout (location = 1) in vec2 UV;\n"
            "layout (location = 2) in vec4 Color;\n"
            "uniform mat4 ProjMtx;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";



    const GLchar* fragment_shader =
            "#version 330 core\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "uniform sampler2D Texture;\n"
            "layout (location = 0) out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";



    void enableGLFeatures() {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);
    }


    GuiRender::GuiRender() {

    }

    GuiRender::~GuiRender() {

    }

    void GuiRender::setup() {
        if(!setupGL()) {
            RB_EDITOR_ERROR("Can't setup Gui Render");
            return;
        }
        setupFonts();
    }

    void GuiRender::setupFonts() {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;

        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        m_fontTexture.create({width, height}, pixels);

        ImTextureID texID = convertTextureHandle(m_fontTexture.getID());
        io.Fonts -> SetTexID(texID);
    }

    bool GuiRender::setupGL()
    {

        GLint last_texture, last_array_buffer;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        GLint last_vertex_array;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

        // Create buffers
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        //glGenVertexArrays(1, &VAO);

        if(!m_shader.createShader(robot2D::shaderType::vertex, vertex_shader, false)) {
            RB_EDITOR_ERROR("Cant' create GUI VertexShader");
            return false;
        }

        if(!m_shader.createShader(robot2D::shaderType::fragment, fragment_shader, false)) {
            RB_EDITOR_ERROR("Cant' create GUI Fragment Shader");
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glBindVertexArray(last_vertex_array);
#endif

        return true;
    }

    void GuiRender::setupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, unsigned int vertex_array_object)
    {
        enableGLFeatures();

        glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

        orthoView.update({draw_data->DisplayPos.x,
                          draw_data->DisplayPos.y},
                         {draw_data->DisplaySize.x,
                          draw_data->DisplaySize.y});


        m_shader.use();
        m_shader.set_parameter("Texture", 0);
        m_shader.set_parameter("ProjMtx", orthoView.getMatrix());


        (void)vertex_array_object;
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glBindVertexArray(vertex_array_object);
#endif

        // Bind vertex/index buffers and setup attributes for ImDrawVert
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
    }


    void GuiRender::render(ImDrawData* draw_data) {

        int fb_width = static_cast<int>(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
        int fb_height = static_cast<int>(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

        if (fb_width <= 0 || fb_height <= 0)
            return;
        // Setup desired GL state
        // Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
        // The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
        GLuint vertex_array_object = 0;


        GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
        GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
        GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
        GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
        GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
        GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
        GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
        GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
        GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
        GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
        GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
        GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
        GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
        GLuint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        GLuint last_vertex_array_object; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
#endif


#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glGenVertexArrays(1, &vertex_array_object);
#endif
        setupRenderState(draw_data, fb_width, fb_height, vertex_array_object);

        // Will project scissor/clipping rectangles into framebuffer space
        ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
        ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        // Render command lists
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];

            // Upload vertex/index buffers
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert),
                         (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx),
                         (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user
                    // to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        setupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
                    else
                        pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                    (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                    (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                    if (clip_max.x < clip_min.x || clip_max.y < clip_min.y)
                        continue;

                    // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                    glScissor((int)clip_min.x,
                              (int)(fb_height - clip_max.y),
                              (int)(clip_max.x - clip_min.x),
                              (int)(clip_max.y - clip_min.y));

                    // Bind texture, Draw
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                                   sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                   (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));

                }
            }
        }
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glDeleteVertexArrays(1, &vertex_array_object);
#endif
        // Restore modified GL state
        m_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fontTexture.getID());

#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glBindVertexArray(last_vertex_array_object);
#endif

        //    glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
        glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);

        if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
        if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);


        glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
        glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
    }

}