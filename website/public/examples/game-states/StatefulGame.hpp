#pragma once

#include "../collect-items/CollectItems.hpp"
#include "GameFlow.hpp"

class StatefulGame : public CollectItems {
public:
    using CollectItems::CollectItems;

    void setup() override {
        CollectItems::setup();
        showPhase();
    }

    GamePhase phase() const { return m_flow.phase(); }

    void handleEvents(const robot2D::Event& event) override {
        // Window/exit events must work even when simulation is frozen.
        if(event.getIf<robot2D::Event::Closed>()) {
            CollisionSprite::handleEvents(event);
            return;
        }
        if(event.getIf<robot2D::Event::LostFocus>()) {
            CollisionSprite::handleEvents(event); // Clears held keys and records input focus.
            m_flow.loseFocus();
            showPhase();
            return;
        }
        if(event.getIf<robot2D::Event::GainFocus>()) {
            CollisionSprite::handleEvents(event);
            m_flow.gainFocus();
            return;
        }
        if(const auto* minimized = event.getIf<robot2D::Event::MinimizedChanged>()) {
            if(minimized -> minimized) { m_flow.pause(); clearMovement(); showPhase(); }
            return;
        }
        if(const auto* key = event.getIf<robot2D::Event::KeyPressed>()) {
            if(key -> code == robot2D::Key::ESCAPE) {
                CollisionSprite::handleEvents(event);
                return;
            }
            if(m_flow.focused() && !key -> repeat) {
                if(key -> code == robot2D::Key::P) {
                    if(m_flow.togglePause()) { clearMovement(); showPhase(); }
                    return;
                }
                if(key -> code == robot2D::Key::R) {
                    restartRound();
                    m_flow.restart();
                    clearMovement();
                    showPhase();
                    return;
                }
            }
        }
        // Do not call CollectItems::handleEvents: its R handling bypasses this game's policy.
        // Releases still reach input; paused/won presses never become latent movement.
        if(m_flow.canUpdate() || event.getIf<robot2D::Event::KeyReleased>())
            CollisionSprite::handleEvents(event);
    }

    void update(float deltaTime) override {
        if(m_window -> getMetrics().drawable()) stepGame(deltaTime);
    }

protected:
    void stepGame(float deltaTime) {
        if(!m_flow.canUpdate()) return;
        const auto previousScore = round().score();
        stepRound(deltaTime);
        if(round().complete()) {
            m_flow.win();
            clearMovement();
            showPhase();
        }
        else if(round().score() != previousScore) {
            showPhase(); // Lesson 07 refreshed its hint; keep this game's controls visible.
        }
    }

private:
    void showPhase() { setHint(m_flow.hint()); }
    GameFlow m_flow;
};
