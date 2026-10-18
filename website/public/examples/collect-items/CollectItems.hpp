#pragma once

#include "../sprite-collisions/CollisionSprite.hpp"
#include "CollectionState.hpp"
#include <memory>
#include <robot2D/Graphics/Text.hpp>

class CollectItems : public CollisionSprite {
public:
    explicit CollectItems(std::string imagePath = "assets/robot.png",
        std::string fontPath = "assets/SourceSansPro-Regular.ttf") :
        CollisionSprite(std::move(imagePath)), m_fontPath(std::move(fontPath)) {}

    void setup() override {
        CollisionSprite::setup();
        const std::array<unsigned char, 4> white{255, 255, 255, 255};
        m_itemTexture.create({1, 1}, white.data());
        configureItems();
        for(auto& item : m_items) {
            item.setTexture(m_itemTexture, {0, 0, 1, 1});
            item.setColor(robot2D::Color(255.F, 205.F, 65.F));
        }

        m_font = std::make_unique<robot2D::Font>();
        if(!m_font -> loadFromFile(m_fontPath, 24))
            throw std::runtime_error("Cannot load font: " + std::filesystem::absolute(m_fontPath).string());
        if(m_font -> getImages().size() != 1)
            throw std::runtime_error("This lesson requires a single-page ASCII font atlas");
        m_fontTexture.create(*m_font -> getImages().front());
        m_scoreText = makeText({40.F, 35.F});
        m_hintText = makeText({40.F, 64.F});
        refreshHud();
    }

    void handleEvents(const robot2D::Event& event) override {
        CollisionSprite::handleEvents(event);
        if(event.getIf<robot2D::Event::LostFocus>()) m_focused = false;
        if(event.getIf<robot2D::Event::GainFocus>()) m_focused = true;
        if(const auto* key = event.getIf<robot2D::Event::KeyPressed>()) {
            if(m_focused && !key -> repeat && key -> code == robot2D::Key::R) restartRound();
        }
    }

    void update(float deltaTime) override {
        if(m_window -> getMetrics().drawable()) stepRound(deltaTime);
    }

    void destroy() override {
        m_hintText.reset();
        m_scoreText.reset(); // Destroy GL buffers while the context and font are alive.
        m_fontTexture.release();
        m_font.reset();
        m_itemTexture.release();
        CollisionSprite::destroy();
    }

    const CollectionState& round() const { return m_round; }

protected:
    void setHint(const std::string& hint) {
        if(m_hintText) m_hintText -> setText(hint);
    }

    void configureItems() {
        const std::array<robot2D::vec2f, CollectionState::itemCount> positions{{
            {220.F, 100.F}, {650.F, 120.F}, {90.F, 450.F}, {300.F, 500.F}, {650.F, 500.F}}};
        for(unsigned i = 0; i < m_items.size(); ++i) {
            m_items[i].setSize({28.F, 28.F});
            m_items[i].setOrigin({0.5F, 0.5F});
            m_items[i].setPosition(positions[i]);
        }
        m_round.reset();
    }

    void stepRound(float deltaTime) {
        simulate(deltaTime); // Resolve solid walls before evaluating pickup triggers.
        if(collectOverlapping(sprite(), m_items, m_round)) refreshHud();
    }

    void restartRound() {
        configureGeometry();
        m_round.reset();
        refreshHud();
    }

    void drawExtras() override {
        for(unsigned i = 0; i < m_items.size(); ++i) {
            if(m_round.active(i)) m_window -> draw(m_items[i]);
        }
        robot2D::RenderStates states;
        states.texture = &m_fontTexture; // Text borrows Font; its atlas is supplied explicitly.
        if(m_scoreText) m_window -> draw(*m_scoreText, states);
        if(m_hintText) m_window -> draw(*m_hintText, states);
    }

private:
    std::unique_ptr<robot2D::Text> makeText(robot2D::vec2f position) {
        auto text = std::make_unique<robot2D::Text>();
        text -> setFont(*m_font);
        text -> setPosition(position);
        text -> setColor(robot2D::Color::White);
        return text;
    }

    void refreshHud() {
        if(m_scoreText) m_scoreText -> setText(m_round.label());
        if(m_hintText) m_hintText -> setText(m_round.complete()
            ? "All collected! Press R to restart." : "WASD/arrows: move | R: restart");
    }

    std::string m_fontPath;
    CollectionState m_round;
    std::array<robot2D::Sprite, CollectionState::itemCount> m_items;
    robot2D::Texture m_itemTexture;
    std::unique_ptr<robot2D::Font> m_font;
    robot2D::Texture m_fontTexture;
    std::unique_ptr<robot2D::Text> m_scoreText;
    std::unique_ptr<robot2D::Text> m_hintText;
    bool m_focused = true;
};
