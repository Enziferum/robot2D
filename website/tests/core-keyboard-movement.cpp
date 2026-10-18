// Observable movement tests against the same application published in the lesson.
#include "../public/examples/keyboard-movement/MovingSprite.hpp"
#include <cmath>
#include <iostream>
#include <limits>

namespace {
    using robot2D::Event;
    using robot2D::Key;

    class Player final : public MovingSprite {
    public:
        Player() { sprite().setPosition({272.F, 172.F}); }
        robot2D::vec2f position() { return sprite().getPosition(); }
    };

    Event press(Key key, bool repeat = false) {
        Event::KeyPressed payload;
        payload.code = key;
        payload.repeat = repeat;
        return Event(payload);
    }

    Event release(Key key) {
        Event::KeyReleased payload;
        payload.code = key;
        return Event(payload);
    }

    void near(float actual, float expected) {
        if(!std::isfinite(actual) || std::abs(actual - expected) > 0.003F)
            throw std::runtime_error("Expected " + std::to_string(expected) + ", got " + std::to_string(actual));
    }

    void expect(Player& player, float x, float y) {
        near(player.position().x, x);
        near(player.position().y, y);
    }

    void held() {
        Player player;
        player.handleEvents(press(Key::D));
        expect(player, 272.F, 172.F); // Events change held state, not position.
        for(int i = 0; i < 10; ++i) player.handleEvents(press(Key::D, true));
        player.update(0.25F);
        player.update(0.25F);
        expect(player, 362.F, 172.F);
        player.handleEvents(release(Key::D));
        player.update(1.F);
        expect(player, 362.F, 172.F);
    }

    void deltaTime() {
        for(int steps : {30, 60, 120}) {
            Player player;
            player.handleEvents(press(Key::D));
            for(int i = 0; i < steps; ++i) player.update(1.F / static_cast<float>(steps));
            expect(player, 452.F, 172.F);
        }
        Player player;
        player.handleEvents(press(Key::D));
        for(float dt : {0.13F, 0.17F, 0.07F, 0.23F, 0.4F}) player.update(dt);
        expect(player, 452.F, 172.F);
        for(float dt : {0.F, -1.F, std::numeric_limits<float>::infinity(),
                std::numeric_limits<float>::quiet_NaN()}) player.update(dt);
        expect(player, 452.F, 172.F);
    }

    void directions() {
        Player diagonal;
        diagonal.handleEvents(press(Key::W));
        diagonal.handleEvents(press(Key::D));
        diagonal.update(1.F);
        const auto offset = diagonal.position() - robot2D::vec2f{272.F, 172.F};
        near(std::sqrt(offset.x * offset.x + offset.y * offset.y), 180.F);
        if(offset.x <= 0.F || offset.y >= 0.F) throw std::runtime_error("Wrong diagonal direction");
        Player aliases;
        aliases.handleEvents(press(Key::A));
        aliases.handleEvents(press(Key::LEFT));
        aliases.handleEvents(release(Key::A));
        aliases.update(0.5F);
        expect(aliases, 182.F, 172.F);
        aliases.handleEvents(press(Key::D));
        aliases.update(1.F);
        expect(aliases, 182.F, 172.F); // Opposing directions cancel.
        aliases.handleEvents(release(Key::LEFT));
        aliases.handleEvents(release(Key::D));
        aliases.handleEvents(press(Key::DOWN));
        aliases.update(0.5F);
        expect(aliases, 182.F, 262.F);
    }

    void focus() {
        Player player;
        player.handleEvents(press(Key::D));
        player.update(0.25F);
        player.handleEvents(Event(Event::LostFocus{}));
        player.handleEvents(press(Key::D, true));
        player.update(1.F);
        expect(player, 317.F, 172.F);
        player.handleEvents(Event(Event::GainFocus{}));
        player.handleEvents(press(Key::D, true));
        player.update(1.F);
        expect(player, 317.F, 172.F); // Regaining focus does not restore stale keys.
        player.handleEvents(press(Key::D));
        player.update(0.5F);
        expect(player, 407.F, 172.F);
    }

    void events() {
        Player player;
        player.handleEvents(press(Key::Unknown));
        player.handleEvents(press(static_cast<Key>(999)));
        Event::TextEntered text;
        text.symbol = 'D';
        player.handleEvents(Event(text));
        player.update(1.F);
        expect(player, 272.F, 172.F);
        player.handleEventFrame({1, {press(Key::D), release(Key::D)}});
        player.update(0.5F);
        expect(player, 272.F, 172.F); // A held-state example does not queue tap actions.
        player.handleEvents(press(Key::ESCAPE));
        if(player.isRunning()) throw std::runtime_error("Escape did not stop application");
        Player closed;
        closed.handleEvents(Event(Event::Closed{}));
        if(closed.isRunning()) throw std::runtime_error("Close event no longer stops application");
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) return 1;
        const std::string scenario = argv[1];
        if(scenario == "held") held();
        else if(scenario == "delta-time") deltaTime();
        else if(scenario == "directions") directions();
        else if(scenario == "focus") focus();
        else if(scenario == "events") events();
        else return 1;
        std::cout << scenario << " passed without a graphics context\n";
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
