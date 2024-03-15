#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/Vertex.hpp>

#include <robot2D/Util/Logger.hpp>

#include <robot2D/imgui/Render.hpp>
#include <robot2D/imgui/Util.hpp>

namespace robot2D {
    #define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY

    const std::string vertexShader = R"(
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

    const std::string fragmentShader = R"(
        #version 330 core
        in vec4 FragColor;
        in vec2 FragUV;

        uniform sampler2D Texture;
        layout (location = 0) out vec4 Out_Color;

        void main() {
            Out_Color = FragColor * texture(Texture, FragUV.st);
        }
    )";

    IRenderContext::~IRenderContext() {}

    struct OpenGLContext: IRenderContext {
        ~OpenGLContext() override = default;

        void enableFeatures() override{
            glEnable(GL_BLEND);
            glEnable(GL_SCISSOR_TEST);

            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_STENCIL_TEST);
        }

        void actualize() override{
            glGetIntegerv(GL_VIEWPORT, last_viewport);
            glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
            glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
            glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
            glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
            glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
            glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
            glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
            last_enable_blend = glIsEnabled(GL_BLEND);
            last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
            last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
            last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
            last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
        }

        void restore() override{
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
    private:
        GLint last_viewport[4];
        GLint last_scissor_box[4];
        GLenum last_blend_src_rgb;
        GLenum last_blend_dst_rgb;
        GLenum last_blend_src_alpha;
        GLenum last_blend_dst_alpha;
        GLenum last_blend_equation_rgb;
        GLenum last_blend_equation_alpha;
        GLboolean last_enable_blend;
        GLboolean last_enable_cull_face;
        GLboolean last_enable_depth_test;
        GLboolean last_enable_stencil_test;
        GLboolean last_enable_scissor_test;
        GLuint last_array_buffer;
    };

    std::unique_ptr<IRenderContext> getRenderContext() {
        return std::make_unique<OpenGLContext>();
    }

    GuiRender::GuiRender()  = default;
    GuiRender::~GuiRender() = default;

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


//        ImFontConfig config;
//        config.SizePixels = 16.F;
//        io.Fonts -> AddFontDefault(&config);


        io.Fonts -> GetTexDataAsRGBA32(&pixels, &width, &height);
        if(!pixels)
            return;
		m_fontTexture.create({static_cast<unsigned int>(width), static_cast<unsigned int>(height)}, pixels);

        ImTextureID texID = ImGui::convertTextureHandle(m_fontTexture.getID());
        io.Fonts -> SetTexID(texID);
    }

    bool GuiRender::setupGL()
    {
        m_renderContext = getRenderContext();
        if(!m_renderContext)
            return false;

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

        bool shaderSourceIsPath = false;
        if(!m_shader.createShader(robot2D::ShaderType::Vertex, vertexShader, shaderSourceIsPath)) {
            RB_EDITOR_ERROR("Can't create GUI VertexShader");
            return false;
        }

        if(!m_shader.createShader(robot2D::ShaderType::Fragment, fragmentShader, shaderSourceIsPath)) {
            RB_EDITOR_ERROR("Can't create GUI Fragment Shader");
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glBindVertexArray(last_vertex_array);
#endif

        m_vertexArray = robot2D::VertexArray::Create();
        m_vertexBuffer = robot2D::VertexBuffer::Create(sizeof(ImDrawVert) * 10000);

        m_vertexBuffer -> setAttributeLayout({
            { robot2D::ElementType::Float2, "Position" },
            { robot2D::ElementType::Float2, "Texture Coords" },
            { robot2D::ElementType::Float4, "Color" },
        });

        m_vertexArray -> setVertexBuffer(m_vertexBuffer);

        return true;
    }

    void GuiRender::setupRenderState(ImDrawData* draw_data, const robot2D::vec2i& framebufferSize,
                                     unsigned int vertex_array_object)
    {
        m_renderContext -> enableFeatures();

        glViewport(0, 0, (GLsizei)framebufferSize.x, (GLsizei)framebufferSize.y);

        orthoView.update({draw_data -> DisplayPos.x,
                          draw_data -> DisplayPos.y,
                          draw_data -> DisplaySize.x,
                          draw_data -> DisplaySize.y});

//        RB_EDITOR_INFO("ImGui Viewport X:{0}, Y:{1}, W:{2}, H:{3}",draw_data -> DisplayPos.x,
//                       draw_data -> DisplayPos.y,
//                       draw_data -> DisplaySize.x,
//                       draw_data -> DisplaySize.y);

        m_shader.use();
        m_shader.set("Texture", 0);
        m_shader.setMatrix("ProjMtx", orthoView.getMatrix());

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
        // Will project scissor/clipping rectangles into framebuffer space
        ImVec2 clip_off = draw_data -> DisplayPos;         // (0,0) unless using multi-viewports
        ImVec2 clip_scale = draw_data -> FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        GLuint vertex_array_object = 0;
        robot2D::vec2i framebufferSize = {
                static_cast<int>(draw_data->DisplaySize.x * draw_data->FramebufferScale.x),
                static_cast<int>(draw_data->DisplaySize.y * draw_data->FramebufferScale.y)
        };

        if (framebufferSize.x <= 0 || framebufferSize.y <= 0)
            return;


        glGenVertexArrays(1, &vertex_array_object);
        m_renderContext -> actualize();
        setupRenderState(draw_data, framebufferSize, vertex_array_object);

        // Render command lists
        for (int n = 0; n < draw_data -> CmdListsCount; n++)
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
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x < clip_min.x || clip_max.y < clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                glScissor((int)clip_min.x,
                          (int)(framebufferSize.y - clip_max.y),
                          (int)(clip_max.x - clip_min.x),
                          (int)(clip_max.y - clip_min.y));

                // Bind texture, Draw
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                               sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                               (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));


            }
        }

        glDeleteVertexArrays(1, &vertex_array_object);

        m_shader.unUse();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fontTexture.getID());

        m_renderContext -> restore();
    }

} // namespace robot2D