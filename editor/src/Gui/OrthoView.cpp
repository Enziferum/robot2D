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

#include <editor/Gui/OrthoView.hpp>

namespace ImGui {
    OrthoView::OrthoView(): currentPos{}, currentSize{},
                 needUpdate{false} {
        matrix[0][0] = 0.F; matrix[1][0] = 0.F; matrix[2][0] = 0.F; matrix[3][0] = 0.F;
        matrix[0][1] = 0.F; matrix[1][1] = 0.F; matrix[2][1] = 0.F; matrix[3][1] = 0.F;
        matrix[0][2] = 0.F; matrix[1][2] = 0.F; matrix[2][2] = -1.F; matrix[3][2] = 0.F;
        matrix[0][3] = 0.F; matrix[1][3] = 0.F; matrix[2][3] = 0.F; matrix[3][3] = 1.F;
    }

    void OrthoView::update(robot2D::vec2f pos, robot2D::vec2f size) {
        if(currentPos != pos || currentSize != size) {
            currentPos = pos;
            currentSize = size;
            needUpdate = true;
        }
    }

    const float* OrthoView::getMatrix() {
        if(!needUpdate)
            return &matrix[0][0];
        float L = currentPos.x;
        float R = currentPos.x + currentSize.x;
        float T = currentPos.y;
        float B = currentPos.y + currentSize.y;

        matrix[0][0] = 2.0F/(R-L);
        matrix[3][0] = (R+L)/(L-R);
        matrix[1][1] = 2.0F/(T-B);
        matrix[3][1] = (T+B)/(B-T);
        needUpdate = false;
        return &matrix[0][0];
    }

}