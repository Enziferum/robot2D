/*********************************************************************
(c) Alex Raag 2023
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

#include <robot2D/Util/Logger.hpp>
#include <editor/SpriteCutter.hpp>


namespace editor {
    namespace {

        int ROW;
        int COL;
    }

    bool SpriteCutter::isSafe(const colorPointMatrix& M, int row, int col, visitMatrix& visited) {
        return (row >= 0) && (row < ROW) && (col >= 0)
               && (col < COL)
               && (!M[row][col] && !visited[row][col]);
    }

    void SpriteCutter::dfs(const colorPointMatrix& M, int row, int col, visitMatrix& visited,
             std::vector<robot2D::vec2i>& points) {

        static int rowNbr[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
        static int colNbr[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

        // Mark this cell as visited
        visited[row][col] = true;
        try {
            points.emplace_back(M[row][col].pos);
        }
        catch(const std::bad_alloc& ex) {
            RB_CRITICAL("Bad alloc");
        }
        for (int k = 0; k < 8; ++k) {
            if (isSafe(M, row + rowNbr[k], col + colNbr[k], visited))
                dfs(M, row + rowNbr[k], col + colNbr[k], visited, points);
        }
    }


    std::vector<robot2D::IntRect>
    SpriteCutter::cutFrames(const robot2D::UIntRect& clipRegion, robot2D::Image& image,
                                                           robot2D::vec2f worldPosition) {
        std::vector<robot2D::IntRect> frames;

        auto* pixelBuffer = image.getBuffer();
        std::vector<std::vector<ColorPoint>> colorPoints;

        auto size = image.getSize();
        robot2D::vec2i localPosition = {static_cast<int>(clipRegion.lx - worldPosition.x),
                                        static_cast<int>(clipRegion.ly - worldPosition.y)};

        constexpr const int channelsNum = 4;
        constexpr const int colorIndex = 4;
        int pixelBufferIndex;
        int sizeDivider = channelsNum * size.x;
        int xOffset = (localPosition.x - 1) * channelsNum;
        {
            for(int i = 0; i < clipRegion.height; ++i) {
                std::vector<ColorPoint> line{static_cast<unsigned int>(clipRegion.width)};
                pixelBufferIndex = (localPosition.y + i) * sizeDivider + xOffset;
                for(int j = 0; j < clipRegion.width; ++j) {
                    pixelBufferIndex += colorIndex;
                    auto pos = robot2D::vec2i{localPosition.x + j, localPosition.y + i};
                    line[j] = {
                            pixelBuffer[pixelBufferIndex + 3]  == 0 ? true : false,
                            pos
                    };
                }
                colorPoints.push_back(line);
            }

            ROW = clipRegion.height;
            COL = clipRegion.width;
        }

        {
            visitMatrix visited(ROW, std::vector<int>(COL));

            int j;
            for (int i = 0; i < ROW; ++i) {
                const auto& line = colorPoints[i];
                for (j = 0; j < COL; ++j) {
                    if (!colorPoints[i][j] && !visited[i][j]) {
                        /// dfs ///
                        std::vector<robot2D::vec2i> points;
                        points.reserve(1000);
                        dfs(colorPoints, i, j, visited, points);
                        if (!points.empty()) {
                            robot2D::vec2i minPoint = points[0];
                            robot2D::vec2i maxPoint = points[0];

                            for(const auto& point : points) {
                                if(minPoint.x > point.x)
                                    minPoint.x = point.x;
                                if(maxPoint.x < point.x)
                                    maxPoint.x = point.x;
                                if(minPoint.y > point.y)
                                    minPoint.y = point.y;
                                if(maxPoint.y < point.y)
                                    maxPoint.y = point.y;
                            }

                            minPoint += worldPosition.as<int>();
                            maxPoint += worldPosition.as<int>();

                            frames.emplace_back(robot2D::IntRect {
                                    minPoint.x,
                                    minPoint.y,
                                    maxPoint.x - minPoint.x + 1,
                                    maxPoint.y - minPoint.y + 1,
                            });
                        }
                    }
                }
            }
        }
        return frames;
    }

} // namespace editor