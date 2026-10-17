#include "CameraSprite.hpp"

int main() {
    robot2D::robot2DInit();
    return robot2D::runEngine(std::make_unique<CameraSprite>(),
        robot2D::EngineConfiguration({800, 600}, "robot2D - Camera and mouse", 60));
}
