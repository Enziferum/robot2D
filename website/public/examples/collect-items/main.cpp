#include "CollectItems.hpp"

int main(int argc, char** argv) {
    robot2D::robot2DInit();
    auto application = std::make_unique<CollectItems>(
        argc > 1 ? argv[1] : "assets/robot.png",
        argc > 2 ? argv[2] : "assets/SourceSansPro-Regular.ttf");
    return robot2D::runEngine(std::move(application),
        robot2D::EngineConfiguration({800, 600}, "robot2D - Items and score", 60));
}
