#include "../public/examples/game-states/StatefulGame.hpp"
#include <iostream>

namespace {
    void require(bool value, const char* message) {
        if(!value) throw std::runtime_error(message);
    }

    robot2D::Event key(robot2D::Key code, bool repeat = false, bool pressed = true) {
        if(pressed) {
            robot2D::Event::KeyPressed payload;
            payload.code = code; payload.repeat = repeat;
            return robot2D::Event(payload);
        }
        robot2D::Event::KeyReleased payload;
        payload.code = code;
        return robot2D::Event(payload);
    }

    class Player final : public StatefulGame {
    public:
        Player() { configureGeometry(); configureItems(); m_running = true; }
        void step(float dt) { stepGame(dt); }
        robot2D::vec2f position() { return sprite().getPosition(); }
        void place(robot2D::vec2f position) { sprite().setPosition(position); }
        void finish() {
            for(auto position : {robot2D::vec2f{220.F, 100.F}, {650.F, 120.F},
                    {90.F, 450.F}, {300.F, 500.F}, {650.F, 500.F}}) {
                place(position); step(0.F);
            }
        }
    };

    void pause() {
        Player player;
        player.handleEvents(key(robot2D::Key::D)); player.step(0.25F);
        const auto saved = player.position();
        player.handleEvents(key(robot2D::Key::P));
        require(player.phase() == GamePhase::Paused, "P must pause");
        for(int i = 0; i < 10; ++i) player.step(10.F);
        require(player.position() == saved && player.round().score() == 0, "Pause changed simulation");
        player.handleEvents(key(robot2D::Key::P)); player.step(0.25F);
        require(player.phase() == GamePhase::Playing && player.position() == saved, "Resume restored stale movement");
        Player pickup;
        pickup.place({220.F, 100.F}); pickup.handleEvents(key(robot2D::Key::P)); pickup.step(1.F);
        require(pickup.round().score() == 0, "Paused trigger awarded points");
        pickup.handleEvents(key(robot2D::Key::P)); pickup.step(0.F);
        require(pickup.round().score() == 10, "Playing trigger failed after resume");
    }

    void input() {
        Player player;
        player.handleEvents(key(robot2D::Key::D));
        player.handleEvents(key(robot2D::Key::P));
        player.handleEvents(key(robot2D::Key::P, true));
        player.handleEvents(key(robot2D::Key::W));
        require(player.phase() == GamePhase::Paused, "Autorepeat toggled pause");
        player.handleEvents(key(robot2D::Key::W, false, false));
        player.handleEvents(key(robot2D::Key::P));
        player.handleEvents(key(robot2D::Key::D, true)); player.step(1.F);
        require(player.position() == robot2D::vec2f{100.F, 100.F}, "Latent paused/repeated input moved player");
        player.handleEvents(key(robot2D::Key::D)); player.step(0.1F);
        require(player.position().x > 100.F && player.position().y == 100.F, "Fresh movement press failed");
    }

    void focus() {
        Player player;
        player.handleEvents(key(robot2D::Key::D)); player.step(0.1F);
        const auto saved = player.position();
        player.handleEvents(robot2D::Event(robot2D::Event::LostFocus{}));
        player.handleEvents(key(robot2D::Key::P)); player.handleEvents(key(robot2D::Key::R)); player.step(1.F);
        require(player.phase() == GamePhase::Paused && player.position() == saved, "Unfocused controls changed round");
        player.handleEvents(robot2D::Event(robot2D::Event::GainFocus{})); player.step(1.F);
        require(player.phase() == GamePhase::Paused, "Focus gain resumed automatically");
        player.handleEvents(key(robot2D::Key::P)); player.step(1.F);
        require(player.position() == saved, "Focus gain restored held keys");
        robot2D::Event::MinimizedChanged minimized; minimized.minimized = true;
        player.handleEvents(robot2D::Event(minimized));
        minimized.minimized = false; player.handleEvents(robot2D::Event(minimized));
        require(player.phase() == GamePhase::Paused, "Restore resumed minimized game");
    }

    void victory() {
        Player player;
        player.finish();
        require(player.phase() == GamePhase::Won && player.round().score() == 50, "Final pickup did not win");
        const auto saved = player.position();
        player.handleEvents(key(robot2D::Key::P)); player.handleEvents(key(robot2D::Key::A));
        for(int i = 0; i < 100; ++i) player.step(1.F);
        require(player.phase() == GamePhase::Won && player.position() == saved && player.round().score() == 50,
            "Won state resumed simulation");
        player.handleEvents(robot2D::Event(robot2D::Event::LostFocus{}));
        player.handleEvents(robot2D::Event(robot2D::Event::GainFocus{}));
        require(player.phase() == GamePhase::Won, "Focus changed victory");
    }

    void restart() {
        for(int source = 0; source < 3; ++source) {
            Player player;
            player.place({220.F, 100.F}); player.step(0.F);
            if(source == 1) player.handleEvents(key(robot2D::Key::P));
            if(source == 2) player.finish();
            player.handleEvents(key(robot2D::Key::D));
            player.handleEvents(key(robot2D::Key::R)); player.step(0.5F);
            require(player.phase() == GamePhase::Playing && player.round().score() == 0 &&
                player.position() == robot2D::vec2f{100.F, 100.F}, "Restart from state failed");
            player.place({220.F, 100.F}); player.step(0.F);
            player.handleEvents(key(robot2D::Key::R, true));
            require(player.round().score() == 10, "Repeated R reset round");
        }
    }

    void exit() {
        for(int state = 0; state < 3; ++state) {
            for(bool close : {false, true}) {
                Player player;
                if(state == 1) player.handleEvents(key(robot2D::Key::P));
                if(state == 2) player.finish();
                require(player.isRunning(), "Test must start running");
                player.handleEvents(close ? robot2D::Event(robot2D::Event::Closed{}) : key(robot2D::Key::ESCAPE));
                require(!player.isRunning(), "Exit ignored outside Playing");
            }
        }
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) return 1;
        const std::string scenario = argv[1];
        if(scenario == "pause") pause();
        else if(scenario == "input") input();
        else if(scenario == "focus") focus();
        else if(scenario == "victory") victory();
        else if(scenario == "restart") restart();
        else if(scenario == "exit") exit();
        else return 1;
        std::cout << scenario << " passed without a graphics context\n";
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
