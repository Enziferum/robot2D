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

#include <memory>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Util/Logger.hpp>
#include "sandbox/Sandbox.hpp"
#include "sandbox/Render2DScene.hpp"


int main() {
    logger::Log::Init();
    robot2D::RenderWindow window{{800, 600}, {"Sandbox"}};

    Sandbox sandbox(window);

    //// Put own scenes here /////
    Render2DScene::Ptr render2DScene = std::make_unique<Render2DScene>(window);
    render2DScene -> setup();
    //// Put own scenes here /////

    sandbox.setScene(std::move(render2DScene));
    sandbox.run();

    return 0;
}