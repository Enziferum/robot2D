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
#include <algorithm>
#include <editor/MessageDispather.hpp>

namespace editor {

    IFunction::~IFunction() {}

    MessageDispatcher::MessageDispatcher(): m_functions{} {}

    void MessageDispatcher::process(robot2D::Message& message) {
        auto findRange = m_functions.equal_range(message.id);
        std::for_each(findRange.first, findRange.second,
                      [&message](const std::pair<const int, std::unique_ptr<IFunction>>& ptr) {
                          ptr.second -> execute(message);
                      });
    }

}