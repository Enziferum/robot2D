// Observable quad corners, animation timing and camera projection; no GL context.
#include "../public/examples/bounded-sprite/BoundedSprite.hpp"
#include "../public/examples/sprite-animation/AtlasAnimation.hpp"
#include "../public/examples/sprite-animation/BeaconAtlas.hpp"
#include "../public/examples/camera-view/Camera.hpp"
#include <iostream>
#include <limits>

namespace {
    void near(float actual, float expected) {
        if(!std::isfinite(actual) || std::abs(actual - expected) > 0.005F)
            throw std::runtime_error("Expected " + std::to_string(expected) + ", got " + std::to_string(actual));
    }

    void corners(robot2D::Sprite& sprite, float left, float top, float right, float bottom) {
        const auto a = sprite.getTransform().transformPoint({0.F, 0.F});
        const auto b = sprite.getTransform().transformPoint({1.F, 1.F});
        near(a.x, left); near(a.y, top); near(b.x, right); near(b.y, bottom);
    }

    void bounds(const std::string& scenario) {
        robot2D::Sprite sprite;
        sprite.setSize({160.F, 100.F});
        for(const auto origin : {robot2D::vec2f{0.F, 0.F}, {0.5F, 0.5F}, {1.F, 1.F}, {0.25F, 0.75F}}) {
            sprite.setOrigin(origin);
            sprite.setPosition({-1000.F, -1000.F});
            keepInside(sprite, {800.F, 600.F});
            corners(sprite, 0.F, 0.F, 160.F, 100.F);
            sprite.setPosition({10000.F, 10000.F});
            keepInside(sprite, {800.F, 600.F});
            corners(sprite, 640.F, 500.F, 800.F, 600.F);
            if(scenario == "resize") {
                keepInside(sprite, {200.F, 150.F});
                corners(sprite, 40.F, 50.F, 200.F, 150.F);
                keepInside(sprite, {0.F, 0.F});
                corners(sprite, 40.F, 50.F, 200.F, 150.F);
                keepInside(sprite, {80.F, 40.F});
                corners(sprite, -40.F, -30.F, 120.F, 70.F);
                keepInside(sprite, {160.F, 100.F});
                corners(sprite, 0.F, 0.F, 160.F, 100.F);
            }
        }
        if(scenario == "edges") {
            sprite.setOrigin({0.5F, 0.5F});
            sprite.setPosition({400.F, 300.F});
            KeyboardMovement input;
            robot2D::Event::KeyPressed press;
            press.code = robot2D::Key::D;
            input.handleEvent(robot2D::Event(press));
            for(int i = 0; i < 1000; ++i) {
                sprite.move(input.displacement(1.F / 60.F));
                keepInside(sprite, {800.F, 600.F});
            }
            corners(sprite, 640.F, 250.F, 800.F, 350.F);
        }
    }

    void animation() {
        AtlasAnimation animation;
        for(unsigned i = 0; i < 12; ++i) {
            if(animation.frame() != i % 4) throw std::runtime_error("Wrong atlas frame");
            const auto rect = animation.rectangle();
            if(rect.lx != static_cast<int>(i % 4) * 64 || rect.width != 64 || rect.height != 64)
                throw std::runtime_error("Wrong texture rectangle");
            animation.advance(0.125F);
        }
        animation.advance(2.25F);
        if(animation.frame() != 2) throw std::runtime_error("Long update lost elapsed time");
        for(float dt : {0.F, -1.F, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()})
            animation.advance(dt);
        if(animation.frame() != 2) throw std::runtime_error("Invalid time changed animation");
        AtlasAnimation split;
        for(int i = 0; i < 15; ++i) split.advance(1.F / 60.F);
        if(split.frame() != 2) throw std::runtime_error("Frame-rate dependent animation");
        const auto pixels = makeBeaconAtlas();
        if(pixels[(20 * 256 + 12) * 4] == pixels[(20 * 256 + 64 + 12) * 4])
            throw std::runtime_error("Atlas frames must visibly differ");
    }

    void camera() {
        robot2D::View view;
        followPlayer(view, {1000.F, 600.F}, {800, 600});
        auto center = view.getTransform().transformPoint({1000.F, 600.F});
        near(center.x, 0.F); near(center.y, 0.F);
        auto world = view.getInverseTransform().transformPoint({-1.F, 1.F});
        near(world.x, 600.F); near(world.y, 300.F);
        followPlayer(view, {80.F, 80.F}, {1600, 600});
        near(view.getCenter().x, 800.F); near(view.getCenter().y, 300.F);
        followPlayer(view, {1920.F, 1120.F}, {800, 600});
        near(view.getCenter().x, 1600.F); near(view.getCenter().y, 900.F);
        followPlayer(view, {80.F, 80.F}, {3000, 600});
        near(view.getCenter().x, 1000.F); // Oversized view centers on the world.
        followPlayer(view, {0.F, 0.F}, {0, 0});
        near(view.getCenter().x, 1000.F);
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) return 1;
        const std::string scenario = argv[1];
        if(scenario == "origins" || scenario == "edges" || scenario == "resize") bounds(scenario);
        else if(scenario == "animation") animation();
        else if(scenario == "camera") camera();
        else return 1;
        std::cout << scenario << " passed\n";
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
