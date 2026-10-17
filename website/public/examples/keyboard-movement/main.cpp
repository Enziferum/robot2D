#include "MovingSprite.hpp"

int main(int argc, char** argv) {
    robot2D::robot2DInit();
    auto application = std::make_unique<MovingSprite>(
        argc > 1 ? argv[1] : "assets/robot.png");
    return robot2D::runEngine(std::move(application),
        robot2D::EngineConfiguration({800, 600}, "robot2D - Keyboard movement", 60));
}
