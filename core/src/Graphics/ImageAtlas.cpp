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

#include "internal/stb_image_write.h"
#include <robot2D/Graphics/ImageAtlas.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D {
    bool ImageAtlas::loadFromFile(const std::string& path, vec2i itemSize) {
        if(!m_atlasImage.loadFromFile(path)) {
            RB_CORE_ERROR("Can't load ImageAtlas by path {0}", path);
            return false;
        }
        m_rectSize = itemSize;
        return true;
    }

    bool ImageAtlas::createAtlas(const std::vector<robot2D::Image>& images,
                                 robot2D::vec2i itemSize,
                                 int itemsPerRow) {
        constexpr int channelsNum = 4;
        int textureIndex = 1;
        int rowIndex = 0;

        robot2D::vec2i textureSize;
        robot2D::vec2u offset{0, 0};

        auto sz = images.size();

        if(itemsPerRow == 0)
            itemsPerRow = static_cast<int>(sz) / 2;

        textureSize.x = itemSize.x * (sz > itemsPerRow ? itemsPerRow : sz);
        textureSize.y = itemSize.y * (sz > itemsPerRow ? (sz % itemsPerRow == 0 ? sz / itemsPerRow
                                                                                : sz / itemsPerRow + 1) : 1);

        std::unique_ptr<unsigned char[]> pixelBuffer =
                std::make_unique<unsigned char[]>(textureSize.x *  textureSize.y * channelsNum);


        if(!pixelBuffer) {
            RB_CORE_ERROR("Can't allocate pixelBuffer for ImageAtlas");
            return false;
        }

        for(const auto& image: images) {
            auto sourceSize = image.getSize();
            auto* source = image.getBuffer().data();

            if(!source) {
                RB_CORE_ERROR("Can't get source's pixelBuffer");
                return false;
            }

            for(int sourceY = 0; sourceY < static_cast<int>(sourceSize.y); ++sourceY) {
                for(int sourceX = 0; sourceX < static_cast<int>(sourceSize.x); ++sourceX) {
                    int from, to;
                    if(sourceSize.x == sourceSize.y) {
                        int fromY = static_cast<int>(sourceY * sourceSize.y * channelsNum);
                        int toY = static_cast<int>(sourceY + offset.y) * textureSize.x * channelsNum;

                        int fromX = (sourceX * channelsNum);
                        int toX = static_cast<int>(sourceX + offset.x) * channelsNum;

                        from = fromY + fromX;
                        to = toY + toX;
                    }
                    else {
                        int fromY = static_cast<int>(sourceY * sourceSize.x * channelsNum);
                        int toY = static_cast<int>(sourceY + offset.y) * textureSize.x * channelsNum;

                        int fromX = (sourceX * channelsNum);
                        int toX = static_cast<int>(sourceX + offset.x) * channelsNum;

                        from = fromY + fromX;
                        to = toY + toX;
                    }

                    for(int channel = 0; channel < channelsNum; ++channel) {
                        pixelBuffer[to + channel] = source[from + channel];
                    }

                    if ((sourceY % 64) == 0) {
                        printf("\r Pack status %d%%....", 100 * sourceY / sourceSize.y);
                    }
                }
            }

            offset.x += sourceSize.x;
            ++textureIndex;
            ++rowIndex;
            if(rowIndex == itemsPerRow) {
                offset.y += sourceSize.y;
                offset.x = 0;
                rowIndex = 0;
            }
        }

        m_rectSize = itemSize;
        if(!m_atlasImage.create(textureSize.as<unsigned >(), pixelBuffer.get(), ImageColorFormat::RGBA)) {
            RB_CORE_ERROR("Can't create image atlas");
            return false;
        }

        return true;
    }

    IntRect ImageAtlas::getRect(int row, int column) const noexcept{
        const auto& imageSize = m_atlasImage.getSize();

        int maxRow = static_cast<int>(imageSize.x) / m_rectSize.x;
        int maxColumn = static_cast<int>(imageSize.y) / m_rectSize.y;

        assert((row < maxRow && column < maxColumn) && "Row and column must be valid");

        return {m_rectSize.x * row, m_rectSize.y * column, m_rectSize.x, m_rectSize.y};
    }

    IntRect ImageAtlas::getRect(vec2i index) const noexcept{
        return getRect(index.x, index.y);
    }

    vec2i ImageAtlas::rectSize() const noexcept{
        return m_rectSize;
    }

    const Image& ImageAtlas::getImage() const {
        return m_atlasImage;
    }

    bool ImageAtlas::saveAtlas(std::string atlasPath) {
        const auto& textureSize = m_atlasImage.getSize();
        assert(m_atlasImage.getSize() != vec2u{} && "Image Atlas can't have zero size");
        assert(m_atlasImage.getBuffer() != nullptr && "Image Atlas can't have zero pixel buffer");

        int err = stbi_write_png(atlasPath.c_str(),
                                 textureSize.x, textureSize.y,
                                 m_atlasImage.getColorFormat(), m_atlasImage.getBuffer(),
                                 textureSize.x * m_atlasImage.getColorFormat());

        if(err == 0) {
            RB_CORE_ERROR("Can't write image by path {0}", atlasPath);
            return false;
        }

        return true;
    }

}