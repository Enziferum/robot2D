#include "../public/examples/sprite-collisions/CollisionSprite.hpp"
#include <iostream>
#include <limits>

namespace {
    void require(bool condition, const char* message) {
        if(!condition) throw std::runtime_error(message);
    }

    void near(float actual, float expected) {
        require(std::isfinite(actual) && std::abs(actual - expected) < 0.005F, "Unexpected position");
    }

    void place(robot2D::Sprite& sprite, float x, float y, float width = 10.F, float height = 10.F) {
        sprite.setSize({width, height});
        sprite.setPosition({x, y});
    }

    void bounds() {
        robot2D::Sprite player;
        place(player, 100.F, 100.F, 80.F, 60.F);
        for(const auto origin : {robot2D::vec2f{0.F, 0.F}, {0.5F, 0.5F}, {1.F, 1.F}, {0.25F, 0.75F}}) {
            player.setOrigin(origin);
            const auto bounds = quadBounds(player);
            const auto first = player.getTransform().transformPoint({0.F, 0.F});
            const auto last = player.getTransform().transformPoint({1.F, 1.F});
            near(bounds.lx, first.x); near(bounds.ly, first.y);
            near(bounds.lx + bounds.width, last.x); near(bounds.ly + bounds.height, last.y);
        }
        require(!overlaps({0.F, 0.F, 10.F, 10.F}, {10.F, 0.F, 10.F, 10.F}), "Touch is not penetration");
        require(overlaps({0.F, 0.F, 10.F, 10.F}, {9.F, 0.F, 10.F, 10.F}), "Missed overlap");
        require(overlaps({0.F, 0.F, 10.F, 10.F}, {2.F, 2.F, 2.F, 2.F}), "Missed contained box");
    }

    void directions() {
        robot2D::Sprite player;
        std::array<robot2D::Sprite, 1> walls;
        place(walls[0], 50.F, 0.F, 10.F, 100.F);
        place(player, 20.F, 20.F);
        auto contact = moveAndSlide(player, {100.F, 0.F}, walls);
        near(player.getPosition().x, 40.F);
        require(contact.blockedX && !overlaps(quadBounds(player), quadBounds(walls[0])), "Right collision failed");
        place(player, 80.F, 20.F);
        moveAndSlide(player, {-100.F, 0.F}, walls);
        near(player.getPosition().x, 60.F);
        place(walls[0], 0.F, 50.F, 100.F, 10.F);
        place(player, 20.F, 20.F);
        contact = moveAndSlide(player, {0.F, 100.F}, walls);
        near(player.getPosition().y, 40.F);
        require(contact.blockedY, "Down collision failed");
        place(player, 20.F, 80.F);
        moveAndSlide(player, {0.F, -100.F}, walls);
        near(player.getPosition().y, 60.F);
    }

    void sliding() {
        robot2D::Sprite player;
        std::array<robot2D::Sprite, 2> walls;
        place(walls[0], 50.F, 0.F, 10.F, 100.F);
        place(walls[1], 0.F, 60.F, 100.F, 10.F);
        place(player, 40.F, 10.F);
        const auto contact = moveAndSlide(player, {10.F, 20.F}, walls);
        near(player.getPosition().x, 40.F); near(player.getPosition().y, 30.F);
        require(contact.blockedX && !contact.blockedY, "Wall should allow tangential motion");
        for(int i = 0; i < 100; ++i) moveAndSlide(player, {1.F, 1.F}, walls);
        near(player.getPosition().x, 40.F); near(player.getPosition().y, 50.F);
        for(auto& wall : walls) require(!overlaps(quadBounds(player), quadBounds(wall)), "Corner penetration");
        moveAndSlide(player, {-10.F, -10.F}, walls);
        near(player.getPosition().x, 30.F); near(player.getPosition().y, 40.F);
    }

    void sweep() {
        robot2D::Sprite player;
        std::array<robot2D::Sprite, 2> walls;
        place(walls[0], 100.F, 0.F, 1.F, 100.F);
        place(walls[1], 50.F, 0.F, 1.F, 100.F);
        for(int order = 0; order < 2; ++order) {
            place(player, 0.F, 20.F);
            moveAndSlide(player, {10000.F, 0.F}, walls);
            near(player.getPosition().x, 40.F);
            place(player, 200.F, 20.F);
            moveAndSlide(player, {-10000.F, 0.F}, walls);
            near(player.getPosition().x, 101.F);
            std::swap(walls[0], walls[1]);
        }
        const auto saved = player.getPosition();
        moveAndSlide(player, {}, walls);
        moveAndSlide(player, {std::numeric_limits<float>::infinity(), 1.F}, walls);
        moveAndSlide(player, {1.F, std::numeric_limits<float>::quiet_NaN()}, walls);
        near(player.getPosition().x, saved.x); near(player.getPosition().y, saved.y);
    }

    class Player final : public CollisionSprite {
    public:
        Player() { configureGeometry(); sprite().setPosition({100.F, 180.F}); }
        void step(float dt) { simulate(dt); }
        robot2D::vec2f position() { return sprite().getPosition(); }
    };

    robot2D::Event press(robot2D::Key key) {
        robot2D::Event::KeyPressed payload;
        payload.code = key;
        return robot2D::Event(payload);
    }

    void application() {
        for(int steps : {30, 60, 120}) {
            Player player;
            player.handleEvents(press(robot2D::Key::D));
            for(int i = 0; i < steps * 2; ++i) player.step(1.F / steps);
            near(player.position().x, 312.F); near(player.position().y, 180.F);
            player.handleEvents(press(robot2D::Key::S));
            player.step(0.1F);
            require(player.position().y > 180.F, "Actual player failed to slide");
            player.handleEvents(robot2D::Event(robot2D::Event::LostFocus{}));
            const auto saved = player.position();
            player.step(10.F);
            near(player.position().x, saved.x); near(player.position().y, saved.y);
            player.handleEvents(press(robot2D::Key::ESCAPE));
            require(!player.isRunning(), "Escape should exit");
        }
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) return 1;
        const std::string scenario = argv[1];
        if(scenario == "bounds") bounds();
        else if(scenario == "directions") directions();
        else if(scenario == "sliding") sliding();
        else if(scenario == "sweep") sweep();
        else if(scenario == "application") application();
        else return 1;
        std::cout << scenario << " passed without a graphics context\n";
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
