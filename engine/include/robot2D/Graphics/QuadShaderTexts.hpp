#pragma once

namespace robot2D {
    namespace OpenGL {
        /// Default OpenGL Vertex Shader using in BatchRender
        /// Check OpenGLRender.cpp
        const char* vertexSource = R"(
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec4 color;
            layout (location = 2) in vec2 textureCoords;
            layout (location = 3) in float textureIndex;
            layout (location = 4) in int a_entityID;

            out vec2 TexCoords;
            out vec4 Color;
            out flat float TexIndex;
            out flat int v_entityID;

            uniform mat4 projection;
            uniform mat4 view;
            uniform bool is3DRender;

            void main()
            {
                TexCoords = textureCoords;
                Color = color;
                TexIndex = textureIndex;
                v_entityID = a_entityID;
                if(!is3DRender) {
                    gl_Position = projection * vec4(position, 1.0);
                }
                else {
                    gl_Position = projection * view * vec4(position, 1.0);
                }
            }
        )";

        /// Default OpenGL Fragment Shader using in BatchRender
        /// Check OpenGLRender.cpp

        const char* fragmentSource = R"(
            layout (location = 0) out vec4 fragColor;
            layout (location = 1) out int o_entityID;

            in vec2 TexCoords;
            in vec4 Color;
            in flat float TexIndex;
            in flat int v_entityID;

            uniform sampler2D textureSamplers[16];

            void main()
            {
                vec4 texColor = Color;

                switch(int(TexIndex))
                {
                    case  0: texColor = Color; break;
                    case  1: texColor *= texture(textureSamplers[ 1], TexCoords); break;
                    case  2: texColor *= texture(textureSamplers[ 2], TexCoords); break;
                    case  3: texColor *= texture(textureSamplers[ 3], TexCoords); break;
                    case  4: texColor *= texture(textureSamplers[ 4], TexCoords); break;
                    case  5: texColor *= texture(textureSamplers[ 5], TexCoords); break;
                    case  6: texColor *= texture(textureSamplers[ 6], TexCoords); break;
                    case  7: texColor *= texture(textureSamplers[ 7], TexCoords); break;
                    case  8: texColor *= texture(textureSamplers[ 8], TexCoords); break;
                    case  9: texColor *= texture(textureSamplers[ 9], TexCoords); break;
                    case 10: texColor *= texture(textureSamplers[10], TexCoords); break;
                    case 11: texColor *= texture(textureSamplers[11], TexCoords); break;
                    case 12: texColor *= texture(textureSamplers[12], TexCoords); break;
                    case 13: texColor *= texture(textureSamplers[13], TexCoords); break;
                    case 14: texColor *= texture(textureSamplers[14], TexCoords); break;
                    case 15: texColor *= texture(textureSamplers[15], TexCoords); break;
                }

                fragColor = texColor;
                o_entityID = v_entityID;
            }
        )";
    }
}