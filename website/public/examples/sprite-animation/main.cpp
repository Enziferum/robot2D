#include "AnimatedSprite.hpp"

int main() {
    robot2D::robot2DInit();
    return robot2D::runEngine(std::make_unique<AnimatedSprite>(),
        robot2D::EngineConfiguration({800, 600}, "robot2D - Atlas animation", 60));
}
