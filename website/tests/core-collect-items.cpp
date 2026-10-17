#include "../public/examples/collect-items/CollectItems.hpp"
#include <iostream>

namespace {
    void require(bool value, const char* message) {
        if(!value) throw std::runtime_error(message);
    }

    void once() {
        CollectionState state;
        require(state.score() == 0 && state.collected() == 0 && !state.complete(), "Wrong initial round");
        require(state.collect(0), "First pickup should succeed");
        for(int i = 0; i < 100; ++i) require(!state.collect(0), "Repeated pickup awarded points");
        require(state.score() == 10 && state.collected() == 1 && !state.active(0), "Pickup state disagrees");
    }

    void overlap() {
        robot2D::Sprite player;
        player.setSize({10.F, 10.F});
        std::array<robot2D::Sprite, CollectionState::itemCount> items;
        for(auto& item : items) { item.setSize({4.F, 4.F}); item.setPosition({100.F, 100.F}); }
        items[0].setPosition({10.F, 0.F}); // Edge contact alone is not an overlap.
        CollectionState state;
        require(collectOverlapping(player, items, state) == 0, "Touch should not collect");
        items[0].setPosition({9.F, 0.F});
        items[1].setPosition({3.F, 3.F});
        require(collectOverlapping(player, items, state) == 2, "Both overlapping items should collect");
        require(collectOverlapping(player, items, state) == 0 && state.score() == 20, "Repeated overlap changed score");
        player.setOrigin({0.5F, 0.5F});
        player.setPosition({100.F, 100.F});
        require(collectOverlapping(player, items, state) == 3 && state.complete(), "Centered origin lost pickups");
    }

    void complete() {
        CollectionState state;
        for(unsigned i : {4U, 1U, 3U, 0U, 2U}) require(state.collect(i), "Pickup order failed");
        require(state.complete() && state.score() == 50 && state.collected() == 5, "Round should complete at 50");
        require(state.label() == "Score: 50    Items: 5/5", "HUD label disagrees with model");
        state.reset();
        require(state.label() == "Score: 0    Items: 0/5", "Reset did not clear score label");
        for(unsigned i = 0; i < CollectionState::itemCount; ++i) require(state.active(i), "Reset lost an item");
    }

    robot2D::Event key(robot2D::Key code, bool pressed = true, bool repeat = false) {
        if(pressed) {
            robot2D::Event::KeyPressed event;
            event.code = code; event.repeat = repeat;
            return robot2D::Event(event);
        }
        robot2D::Event::KeyReleased event;
        event.code = code;
        return robot2D::Event(event);
    }

    class Player final : public CollectItems {
    public:
        Player() { configureGeometry(); configureItems(); }
        void step(float dt) { stepRound(dt); }
        robot2D::vec2f position() { return sprite().getPosition(); }
        void place(robot2D::vec2f position) { sprite().setPosition(position); }
    };

    void restart() {
        Player player;
        player.place({220.F, 100.F}); player.step(0.F);
        require(player.round().score() == 10, "Application failed to collect");
        player.handleEvents(key(robot2D::Key::R, true, true));
        require(player.round().score() == 10, "Autorepeat restarted the round");
        player.handleEvents(robot2D::Event(robot2D::Event::LostFocus{}));
        player.handleEvents(key(robot2D::Key::R));
        require(player.round().score() == 10, "Unfocused key restarted the round");
        player.handleEvents(robot2D::Event(robot2D::Event::GainFocus{}));
        player.handleEvents(key(robot2D::Key::R));
        require(player.round().score() == 0 && player.position() == robot2D::vec2f{100.F, 100.F}, "Restart failed");
        player.place({220.F, 100.F}); player.step(0.F);
        require(player.round().score() == 10, "Reset item could not be collected again");
        player.handleEvents(key(robot2D::Key::ESCAPE));
        require(!player.isRunning(), "Escape no longer exits");
    }

    void route() {
        for(int rate : {30, 60, 120}) {
            Player player;
            const std::array<robot2D::vec2f, 6> waypoints{{
                {650.F, 100.F}, {650.F, 260.F}, {460.F, 260.F},
                {460.F, 500.F}, {650.F, 500.F}, {90.F, 500.F}}};
            for(const auto target : waypoints) {
                for(int axis = 0; axis < 2; ++axis) {
                    const float start = axis == 0 ? player.position().x : player.position().y;
                    const float goal = axis == 0 ? target.x : target.y;
                    const auto direction = axis == 0 ? (goal > start ? robot2D::Key::D : robot2D::Key::A)
                        : (goal > start ? robot2D::Key::S : robot2D::Key::W);
                    player.handleEvents(key(direction));
                    unsigned updates = 0;
                    while(std::abs((axis == 0 ? player.position().x : player.position().y) - goal) > 180.F / rate) {
                        player.step(1.F / rate);
                        require(++updates < 1000, "Route blocked by a wall");
                    }
                    player.handleEvents(key(direction, false));
                }
            }
            require(player.round().complete() && player.round().score() == 50, "Not all five items reachable");
            for(int i = 0; i < 100; ++i) player.step(1.F / rate);
            require(player.round().score() == 50, "Completed round kept scoring");
        }
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) return 1;
        const std::string scenario = argv[1];
        if(scenario == "once") once();
        else if(scenario == "overlap") overlap();
        else if(scenario == "complete") complete();
        else if(scenario == "restart") restart();
        else if(scenario == "route") route();
        else return 1;
        std::cout << scenario << " passed without a graphics context\n";
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
