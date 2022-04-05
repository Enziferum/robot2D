#pragma once

namespace robot2D {
    namespace OpenGL {
        /// Default OpenGL Vertex Shader using in BatchRender
        /// Check OpenGLRender.cpp
        const char* vertexSource = R"(
            layout (location = 0) in vec2 position;
            layout (location = 1) in vec4 color;
            layout (location = 2) in vec2 textureCoords;
            layout (location = 3) in float textureIndex;
            out vec2 TexCoords;
            out vec4 Color;
            out float TexIndex;
            uniform mat4 projection;
            void main()
            {
                TexCoords = textureCoords;
                Color = color;
                TexIndex = textureIndex;
                gl_Position = projection * vec4(position, 0.0, 1.0);
            }
        )";

        /// Default OpenGL Fragment Shader using in BatchRender
        /// Check OpenGLRender.cpp
        const char* fragmentSource = R"(
            layout (location = 0) out vec4 fragColor;
            in vec2 TexCoords;
            in vec4 Color;
            in float TexIndex;
            uniform sampler2D textureSamplers[16];

            void main()
            {
                int index = int(TexIndex);
                fragColor = texture(textureSamplers[index], TexCoords) * Color;
            }
        )";
    }
}