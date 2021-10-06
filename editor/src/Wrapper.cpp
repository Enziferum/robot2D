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

#include <cstring>
#include <robot2D/Graphics/GL.hpp>
#include <GLFW/glfw3.h>

#include <editor/Wrapper.hpp>

namespace ImGui {
    #define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY

    static const char* Glfw_GetClipboardText(void* user_data)
    {
        return glfwGetClipboardString((GLFWwindow*)user_data);
    }

    static void Glfw_SetClipboardText(void* user_data, const char* text)
    {
        glfwSetClipboardString((GLFWwindow*)user_data, text);
    }


    Wrapper::Wrapper(): m_window(nullptr) {}

    Wrapper::~Wrapper() {
        shutdown();
    }

    ImTextureID convertTextureHandle(const unsigned int& handle) {
        ImTextureID textureID = (ImTextureID) nullptr;
        std::memcpy(&textureID, &handle, sizeof(unsigned int));
        return textureID;
    }

    void Wrapper::setupFonts() {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;

        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        m_fontTexture.create({width, height}, pixels);

        ImTextureID texID = convertTextureHandle(m_fontTexture.getID() );
        io.Fonts->SetTexID(texID);
    }

    void Wrapper::init(robot2D::Window& window) {
        m_window = &window;
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        io.BackendPlatformName = "imgui_impl_robot2D";
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.KeyMap[ImGuiKey_Tab] = robot2D::key2Int(robot2D::Key::TAB);
        io.KeyMap[ImGuiKey_LeftArrow] = robot2D::key2Int(robot2D::Key::LEFT);
        io.KeyMap[ImGuiKey_RightArrow] =robot2D::key2Int(robot2D::Key::RIGHT);
        io.KeyMap[ImGuiKey_UpArrow] = robot2D::key2Int(robot2D::Key::UP);
        io.KeyMap[ImGuiKey_DownArrow] = robot2D::key2Int(robot2D::Key::DOWN);
        io.KeyMap[ImGuiKey_PageUp] = robot2D::key2Int(robot2D::Key::PAGE_UP);
        io.KeyMap[ImGuiKey_PageDown] = robot2D::key2Int(robot2D::Key::PAGE_DOWN);
        io.KeyMap[ImGuiKey_Home] = robot2D::key2Int(robot2D::Key::HOME);
        io.KeyMap[ImGuiKey_End] = robot2D::key2Int(robot2D::Key::END);
        io.KeyMap[ImGuiKey_Insert] = robot2D::key2Int(robot2D::Key::INSERT);
        io.KeyMap[ImGuiKey_Delete] = robot2D::key2Int(robot2D::Key::DEL);
        io.KeyMap[ImGuiKey_Backspace] = robot2D::key2Int(robot2D::Key::BACKSPACE);
        io.KeyMap[ImGuiKey_Space] = robot2D::key2Int(robot2D::Key::SPACE);
        io.KeyMap[ImGuiKey_Enter] = robot2D::key2Int(robot2D::Key::ENTER);
        io.KeyMap[ImGuiKey_Escape] = robot2D::key2Int(robot2D::Key::ESCAPE);
        io.KeyMap[ImGuiKey_A] = robot2D::key2Int(robot2D::Key::A);
        io.KeyMap[ImGuiKey_C] = robot2D::key2Int(robot2D::Key::C);
        io.KeyMap[ImGuiKey_V] = robot2D::key2Int(robot2D::Key::V);
        io.KeyMap[ImGuiKey_X] = robot2D::key2Int(robot2D::Key::X);
        io.KeyMap[ImGuiKey_Y] = robot2D::key2Int(robot2D::Key::Y);
        io.KeyMap[ImGuiKey_Z] = robot2D::key2Int(robot2D::Key::Z);

        io.ClipboardUserData = (GLFWwindow*)m_window->getRaw();
        io.GetClipboardTextFn = Glfw_GetClipboardText;
        io.SetClipboardTextFn = Glfw_SetClipboardText;

        auto sz = window.getSize();
        io.DisplaySize = ImVec2(sz.x, sz.y);

        m_cursors[ImGuiMouseCursor_Arrow].createDefault();
        m_cursors[ImGuiMouseCursor_TextInput].create(robot2D::CursorType::TextInput);
        m_cursors[ImGuiMouseCursor_ResizeNS].create(robot2D::CursorType::ResizeUpDown);
        m_cursors[ImGuiMouseCursor_ResizeEW].create(robot2D::CursorType::ResizeLeftRight);
        m_cursors[ImGuiMouseCursor_Hand].create(robot2D::CursorType::Hand);
        m_cursors[ImGuiMouseCursor_ResizeAll].createDefault();
        m_cursors[ImGuiMouseCursor_ResizeNESW].createDefault();;
        m_cursors[ImGuiMouseCursor_ResizeNWSE].createDefault();
        m_cursors[ImGuiMouseCursor_NotAllowed].createDefault();


        createDeviceObjects();
        setupFonts();
    }


    void Wrapper::handleEvents(const robot2D::Event &event) {

        if(m_windowHasFocus) {
            ImGuiIO& io = ImGui::GetIO();
            switch(event.type) {
                case robot2D::Event::MouseMoved:
                    m_mouseMoved = true;
                    break;
                case robot2D::Event::MousePressed:
                case robot2D::Event::MouseReleased: {
                    int button = event.mouse.btn;
                    if (event.type == robot2D::Event::MousePressed && button >= 0 && button < 3) {
                        m_mousePressed[event.mouse.btn] = true;
                    }
                } break;
                case robot2D::Event::MouseWheel:
                    io.MouseWheelH += event.wheel.scroll_x;
                    io.MouseWheel += event.wheel.scroll_y;
                    break;
                case robot2D::Event::KeyPressed:
                case robot2D::Event::KeyReleased: {
                    int key = robot2D::key2Int(event.key.code);
                    if(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown)) {
                        io.KeysDown[key] = (event.type == robot2D::Event::KeyPressed);
                    }

                    io.KeyCtrl = io.KeysDown[robot2D::key2Int(robot2D::Key::LEFT_CONTROL)]
                            || io.KeysDown[robot2D::key2Int(robot2D::Key::RIGHT_CONTROL)];
                    io.KeyShift = io.KeysDown[robot2D::key2Int(robot2D::Key::LEFT_SHIFT)]
                            || io.KeysDown[robot2D::key2Int(robot2D::Key::RIGHT_SHIFT)];
                    io.KeyAlt = io.KeysDown[robot2D::key2Int(robot2D::Key::LEFT_ALT)]
                            || io.KeysDown[robot2D::key2Int(robot2D::Key::RIGHT_ALT)];
#ifdef _WIN32
                    io.KeySuper = false;
#else
                   // io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
#endif
                } break;
                case robot2D::Event::TextEntered:
                    io.AddInputCharacter(event.text.symbol);
                    break;
                default:
                    break;
            }
        }

        switch(event.type) {
            case robot2D::Event::LostFocus: {
                ImGuiIO& io = ImGui::GetIO();
                for(int it = 0; it < IM_ARRAYSIZE(io.KeysDown); ++it) {
                    io.KeysDown[it] = false;
                }
                io.KeyCtrl = false;
                io.KeyAlt = false;
                io.KeyShift = false;
                io.KeySuper = false;
                m_windowHasFocus = false;
            } break;
            case robot2D::Event::GainFocus:
                m_windowHasFocus = true;
                break;
            default:
                break;
        }
    }

    void Wrapper::update(float dt) {
        ImGuiIO& io = ImGui::GetIO();
        updateMouseCursor();

        auto sz = m_window -> getSize();
        io.DisplaySize = ImVec2(sz.x, sz.y);
        io.DeltaTime = dt;
        auto glfwWindow = (GLFWwindow*)m_window->getRaw();

        if(m_windowHasFocus) {
            if(io.WantSetMousePos) {
                robot2D::vec2i newMousePos = {io.MousePos.x, io.MousePos.y};
                glfwSetCursorPos(glfwWindow, newMousePos.x, newMousePos.y);
            } else {
                double x, y;
                glfwGetCursorPos(glfwWindow, &x, &y);
                io.MousePos = ImVec2(x, y);
            }
            for (unsigned int i = 0; i < 3; i++) {
                io.MouseDown[i] = m_mousePressed[i] ||
                        (glfwGetMouseButton(glfwWindow, i) == GLFW_PRESS);
                m_mousePressed[i] = false;
            }
        }

        if(io.MouseDrawCursor) {
            m_window->setMouseCursorVisible(false);
        }

        ImGui::NewFrame();
    }


    void enableGLFeatures() {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);
    }

    //// TODO move OpenGL Render to Robot2D Render system ////

    void Wrapper::setupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, unsigned int vertex_array_object)
    {
        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);

        // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
#if defined(GL_CLIP_ORIGIN)
        bool clip_origin_lower_left = true;
//        if (bd->HasClipOrigin)
//        {
//            GLenum current_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&current_clip_origin);
//            if (current_clip_origin == GL_UPPER_LEFT)
//                clip_origin_lower_left = false;
//        }
#endif

        // Setup viewport, orthographic projection matrix
        // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
        glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
#if defined(GL_CLIP_ORIGIN)
        if (!clip_origin_lower_left) { float tmp = T; T = B; B = tmp; } // Swap top and bottom if origin is upper left
#endif
        const float ortho_projection[4][4] =
                {
                        { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
                        { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
                        { 0.0f,         0.0f,        -1.0f,   0.0f },
                        { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
                };

        m_shader.use();
        m_shader.set_parameter("Texture", 0);
        m_shader.set_parameter("ProjMtx", &ortho_projection[0][0]);

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
        if (bd->GlVersion >= 330)
        glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

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

        glVertexAttribPointer(0,   2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        glVertexAttribPointer(1,    2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
    }

    bool Wrapper::createDeviceObjects()
    {
        // Backup GL state
        GLint last_texture, last_array_buffer;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        GLint last_vertex_array;
         glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

        const GLchar* vertex_shader_glsl_410_core =
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



        const GLchar* fragment_shader_glsl_410_core =
                "#version 330 core\n"
                "in vec2 Frag_UV;\n"
                "in vec4 Frag_Color;\n"
                "uniform sampler2D Texture;\n"
                "layout (location = 0) out vec4 Out_Color;\n"
                "void main()\n"
                "{\n"
                "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
                "}\n";



        m_shader.createShader(robot2D::shaderType::vertex, vertex_shader_glsl_410_core, false);
        m_shader.createShader(robot2D::shaderType::fragment, fragment_shader_glsl_410_core, false);

        // Create buffers
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);


        // Restore modified GL state
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glBindVertexArray(last_vertex_array);
#endif

        return true;
    }

    void Wrapper::render() {
        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();
        int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
        int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

        if (fb_width <= 0 || fb_height <= 0)
            return;


#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
        GLuint last_sampler; if (bd->GlVersion >= 330) { glGetIntegerv(GL_SAMPLER_BINDING, (GLint*)&last_sampler); }
        else { last_sampler = 0; }
#endif
        GLuint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        GLuint last_vertex_array_object; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
#endif
#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
        GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
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
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
        GLboolean last_enable_primitive_restart = (bd->GlVersion >= 310) ? glIsEnabled(GL_PRIMITIVE_RESTART) : GL_FALSE;
#endif

        // Setup desired GL state
        // Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
        // The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
        GLuint vertex_array_object = 0;
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
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        setupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
                    else
                        pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                    if (clip_max.x < clip_min.x || clip_max.y < clip_min.y)
                        continue;

                    // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                    glScissor((int)clip_min.x,
                              (int)(fb_height - clip_max.y),
                              (int)(clip_max.x - clip_min.x),
                              (int)(clip_max.y - clip_min.y));

                    // Bind texture, Draw
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
                    if (bd->GlVersion >= 320)
                    glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                                             sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
                else
#endif
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                                   sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                   (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
                }
            }
        }

        // Destroy the temporary VAO
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glDeleteVertexArrays(1, &vertex_array_object);
#endif

        // Restore modified GL state
        m_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fontTexture.getID());

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
        if (bd->GlVersion >= 330)
        glBindSampler(0, last_sampler);
#endif
        //glActiveTexture(last_active_texture);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
        glBindVertexArray(last_vertex_array_object);
#endif
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
        glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
        if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
        if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
        if (bd->GlVersion >= 310) { if (last_enable_primitive_restart) glEnable(GL_PRIMITIVE_RESTART); else glDisable(GL_PRIMITIVE_RESTART); }
#endif

#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
        glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
        glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
        glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
    }

    //// TODO move OpenGL Render to Robot2D Render system ////


    void Wrapper::shutdown() {
        ImGui::GetIO().Fonts->SetTexID(0);
        ImGui::DestroyContext();
    }

    void Wrapper::updateMouseCursor() {
        ImGuiIO& io = ImGui::GetIO();
        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0) {
            ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
            if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None) {
                m_window -> setMouseCursorVisible(false);
            } else {
                m_window -> setMouseCursorVisible(true);
                if(cursor < ImGuiMouseCursor_COUNT)
                    m_window -> setCursor(m_cursors[cursor]);
                else
                    m_window -> setCursor(m_cursors[ImGuiMouseCursor_Arrow]);
            }
        }
    }


    void Image(robot2D::Sprite& sprite, const robot2D::vec2f &size) {
        const robot2D::Texture* sprTexture = sprite.getTexture();
        if(!sprTexture || size == robot2D::vec2f{})
            return;
        const robot2D::Texture& texture = *sprTexture;
        robot2D::vec2f textureSize = {texture.getSize().x, texture.getSize().y};
        auto textureRect = sprite.getLocalBounds();

        auto imID = convertTextureHandle(texture.getID());
        ImVec2 uv0(textureRect.lx / textureSize.x, textureRect.ly / textureSize.y);
        ImVec2 uv1((textureRect.lx + textureRect.width) / textureSize.x,
                   (textureRect.ly + textureRect.height) / textureSize.y);
        ImGui::Image(imID, ImVec2(size.x, size.y), uv0, uv1);
    }

    void RenderFrameBuffer(const robot2D::FrameBuffer::Ptr& frameBuffer, const robot2D::vec2f &size) {
        auto imID = convertTextureHandle(frameBuffer -> getFrameBufferRenderID());
        ImGui::Image(imID, ImVec2(size.x, size.y), {0,1}, {1, 0});
    }

}