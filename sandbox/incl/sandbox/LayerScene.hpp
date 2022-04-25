/*********************************************************************
(c) Alex Raag 2022
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

#include <vector>
#include "Scene.hpp"

class Layer: public robot2D::Drawable {
public:
    Layer();
    ~Layer();

    void draw(const robot2D::Drawable& drawable,
              const robot2D::RenderStates& states = robot2D::RenderStates::Default);
    void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;
private:
};


class LayerScene final: public Scene {
public:
    LayerScene(robot2D::RenderWindow& window);
    ~LayerScene() override;

    void setup() override;
    void handleEvents(const robot2D::Event &event) override;
    void update(float dt) override;
    void imGuiRender() override;
    void render() override;

private:
    std::vector<Layer> m_layers;
};