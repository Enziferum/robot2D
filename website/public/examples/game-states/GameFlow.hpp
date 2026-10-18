// Core lesson 08. Zlib license; see LICENSE.txt.
#pragma once

enum class GamePhase { Playing, Paused, Won };

class GameFlow {
public:
    GamePhase phase() const { return m_phase; }
    bool focused() const { return m_focused; }
    bool canUpdate() const { return m_focused && m_phase == GamePhase::Playing; }

    bool togglePause() {
        if(!m_focused || m_phase == GamePhase::Won) return false;
        m_phase = m_phase == GamePhase::Playing ? GamePhase::Paused : GamePhase::Playing;
        return true;
    }

    void pause() {
        if(m_phase == GamePhase::Playing) m_phase = GamePhase::Paused;
    }

    void loseFocus() { m_focused = false; pause(); }
    void gainFocus() { m_focused = true; } // Returning to the window never resumes automatically.

    void win() {
        if(canUpdate()) m_phase = GamePhase::Won;
    }

    void restart() {
        if(m_focused) m_phase = GamePhase::Playing;
    }

    const char* hint() const {
        switch(m_phase) {
            case GamePhase::Playing: return "Playing - P: pause | R: restart";
            case GamePhase::Paused: return "Paused - P: resume | R: restart";
            case GamePhase::Won: return "Victory! R: restart";
        }
        return "";
    }

private:
    GamePhase m_phase = GamePhase::Playing;
    bool m_focused = true;
};
