#include <robot2D/Util/Logger.hpp>
#include <robot2D/Graphics/Font.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace robot2D {

    constexpr unsigned int TextureAtlasSize = 512;

    Font::Font() = default;

    Font::~Font() {
        auto face = static_cast<FT_Face>(m_face);
        if(face)
            FT_Done_Face(face);

        auto library = static_cast<FT_Library>(m_library);
        if(library)
            FT_Done_FreeType(library);

        m_face = nullptr;
        m_library = nullptr;
    }

    const std::unordered_map<int, GlyphCharacter>& Font::getGlyphCharacters() const {
        return m_glyphCharacters;
    }

    const std::vector<std::unique_ptr<robot2D::Texture>>& Font::getTextures() const {
        return m_textures;
    }

    bool Font::loadFromFile(const std::string& path, int charSize) {
        if(!setup(path, charSize))
            return false;
        setupAtlas();
        return true;
    }

    bool Font::setup(const std::string& path, int charSize) {
        FT_Library library;
        FT_Face face;

        if (FT_Init_FreeType(&library)) {
            RB_CORE_ERROR("ERROR::FREETYPE: Could not init FreeType Library");
            return false;
        }

        if (FT_New_Face(library, path.c_str(), 0, &face)) {
            RB_CORE_ERROR("ERROR::FREETYPE: Failed to load font");
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, charSize);

        m_library = library;
        m_face = face;

        return true;
    }

    void Font::setupAtlas() {
        unsigned int CHARACTERS_TEXTURE_SIZE = TextureAtlasSize;

        auto face = (FT_Face)m_face;
        assert(face != nullptr && "Font Face can't be nullptr");

        std::vector<unsigned char> textureData(CHARACTERS_TEXTURE_SIZE * CHARACTERS_TEXTURE_SIZE, 0);
        std::unique_ptr<robot2D::Texture> texture = std::make_unique<robot2D::Texture>();

        auto currentPixelPositionRow = 0;
        auto currentPixelPositionCol = 0;
        auto rowHeight = 0;
        auto currentRenderIndex = 0;

        auto finalizeTexture = [this,  &texture, &textureData](bool createNext)
        {
            texture -> create({TextureAtlasSize, TextureAtlasSize},
                              textureData.data(), 1, robot2D::ImageColorFormat::RED);
            m_textures.push_back(std::move(texture));
            if (createNext)
            {
                texture = std::make_unique<robot2D::Texture>();
                memset(textureData.data(), 0, textureData.size());
            }
        };

        // Character Range //
        unsigned int start = 32;
        unsigned int end = 127;

        for (auto it = start; it <= end;)
        {
            FT_Load_Glyph(face, FT_Get_Char_Index(face, it), FT_LOAD_DEFAULT);
            FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

            const auto* ptrBitmap = &face->glyph->bitmap;
            const int bmpWidth = static_cast<int>(ptrBitmap->width);
            const int bmpHeight = static_cast<int>(ptrBitmap->rows);
            const auto rowsLeft = CHARACTERS_TEXTURE_SIZE - currentPixelPositionRow;
            const auto colsLeft = CHARACTERS_TEXTURE_SIZE - currentPixelPositionCol;

            rowHeight = std::max(rowHeight, static_cast<int>(bmpHeight));

            if (colsLeft < bmpWidth)
            {
                currentPixelPositionCol = 0;
                currentPixelPositionRow += rowHeight + 1;
                rowHeight = 0;
                continue;
            }

            if (rowsLeft < bmpHeight)
            {
                finalizeTexture(true);
                currentPixelPositionCol = 0;
                currentPixelPositionRow = 0;
                rowHeight = 0;
                continue;
            }

            auto& charProps = m_glyphCharacters[static_cast<int>(it)];
            charProps.size = {
                    static_cast<float>(face -> glyph-> metrics.width >> 6),
                    static_cast<float>(face -> glyph-> metrics.height >> 6)
            };

            charProps.bearing = {
                    static_cast<int>(face->glyph->metrics.horiBearingX >> 6),
                    static_cast<int>(face->glyph->metrics.horiBearingY >> 6)
            };

            charProps.advance = static_cast<float>(face->glyph->metrics.horiAdvance >> 6);

            if (bmpWidth == 0 && bmpHeight == 0) {
                charProps.textureIndex = -1;
                ++it;
                continue;
            }

            for (auto i = 0; i < bmpHeight; i++)
            {
                int globalRow = currentPixelPositionRow + i;
                int reversedRow = bmpHeight - i - 1;
                memcpy(textureData.data() + globalRow * 512 + currentPixelPositionCol,
                       ptrBitmap->buffer + reversedRow * bmpWidth, bmpWidth);
            }


            charProps.bmpSize = { static_cast<float>(bmpWidth),
                                  static_cast<float>(bmpHeight) };

            charProps.indexPosition = {static_cast<float>(currentPixelPositionCol),
                                       static_cast<float>(currentPixelPositionRow) };
            charProps.textureIndex = static_cast<float>(m_textures.size());

            currentPixelPositionCol += bmpWidth + 1;
            currentRenderIndex += 4;
            ++it;
        }

        // If there is a leftover texture after preparing the characters, add it to the list of textures
        if (currentPixelPositionRow > 0 || currentPixelPositionCol > 0) {
            finalizeTexture(false);
        }
    }

    std::unordered_map<int, GlyphQuad> Font::getBufferCache(float scale) const  {
        std::unordered_map<int, GlyphQuad> m_bufferCache;

        float maxBearing = static_cast<float>(m_glyphCharacters.at('H').bearing.y);
        float textureSize = TextureAtlasSize;

        // OpenGL Texture Coords 3210
        // Real World Texture Coords 0123
        for(const auto& glyphCharacter: m_glyphCharacters) {
            const auto& glyphProperties = glyphCharacter.second;

            float x = static_cast<float>(glyphProperties.bearing.x) * scale;
            float y = (maxBearing - static_cast<float>(glyphProperties.bearing.y)) * scale;
            float w = glyphProperties.bmpSize.x * scale;
            float h = glyphProperties.bmpSize.y * scale;

            GlyphVertex vertex0;
            GlyphVertex vertex1;
            GlyphVertex vertex2;
            GlyphVertex vertex3;

            vertex0.position = { x, y };
            vertex0.uvs = {
                    (glyphProperties.indexPosition.x) / textureSize,
                    (glyphProperties.indexPosition.y + glyphProperties.bmpSize.y) / textureSize
            };
            vertex1.position = {x + w, y};
            vertex1.uvs = {
                    (glyphProperties.indexPosition.x + glyphProperties.bmpSize.x) / textureSize,
                    (glyphProperties.indexPosition.y + glyphProperties.bmpSize.y) / textureSize
            };

            vertex2.position = {x + w, y + h};
            vertex2.uvs = {
                    (glyphProperties.indexPosition.x + glyphProperties.bmpSize.x) / textureSize,
                    (glyphProperties.indexPosition.y) / textureSize
            };
            vertex3.position = { x, y + h};
            vertex3.uvs = {
                    glyphProperties.indexPosition.x / textureSize,
                    (glyphProperties.indexPosition.y) / textureSize
            };


            GlyphQuad glyphQuad;
            glyphQuad.vertices[0] = vertex0;
            glyphQuad.vertices[1] = vertex1;
            glyphQuad.vertices[2] = vertex2;
            glyphQuad.vertices[3] = vertex3;

            m_bufferCache.insert(std::pair<int, GlyphQuad>(glyphCharacter.first, glyphQuad));
        }

        return m_bufferCache;
    }

    vec2f Font::calculateSize(std::string&& text) const {
        if(text.empty() || m_glyphCharacters.empty())
            return {};

        vec2f textSize{};
        for(const auto& c: text) {
            auto& currGlyph = m_glyphCharacters.at(c);
            textSize.x += currGlyph.advance;
            textSize.y = std::max(textSize.y, currGlyph.bmpSize.y);
        }

        return textSize;
    }
}