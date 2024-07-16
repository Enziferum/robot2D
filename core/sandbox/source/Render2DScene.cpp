/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <robot2D/Graphics/Graphics.hpp>
#include <sandbox/Render2DScene.hpp>
#include <sandbox/Systems.hpp>
#include <sandbox/Components.hpp>
#include <iomanip>
#include <GLFW/glfw3.h>
#include <cmath>

namespace {
    constexpr robot2D::vec2f position = robot2D::vec2f {100.F, 100.F};
    constexpr robot2D::vec2f size = robot2D::vec2f {100.F, 100.F};
    constexpr const char* texturePath = "res/textures/old_logo.png";
}

struct Quad: robot2D::Drawable, robot2D::Transformable {
    robot2D::Texture* texture = nullptr;
    robot2D::Color color = robot2D::Color::White;

    void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override {
        states.transform *= getTransform();
        states.texture = texture;
        states.color = color;
        target.draw(states);
    }


    robot2D::FloatRect calculateRect() const {
        robot2D::FloatRect rect;
        rect.lx = m_pos.x - m_size.x * m_origin.x;
        rect.ly = m_pos.y - m_size.y * m_origin.y;
        rect.width = m_size.x;
        rect.height = m_size.y;
        return rect;
    }

    robot2D::FloatRect getLocalBounds() const {
        float w = std::abs(m_size.x);
        float h = std::abs(m_size.y);
        auto origin = getOrigin();
        if(origin == robot2D::vec2f{})
            return {0.f, 0.f, w, h};
        /// TODO(a.raag) Make calcs
        float xCoof = 2.f;
        float yCoff = 2.f;
        return {-w / xCoof, -h / yCoff, w, h};
    }

    robot2D::FloatRect getGlobalBounds() const {
        return getTransformNoScale().transformRect(getLocalBounds());
    }
};


struct BoundingBox: robot2D::Drawable {
    struct Wall: robot2D::Drawable{
        robot2D::Transform getTransform() const{
            float angl  = -angle * 3.141592654f / 180.f;
            float cosine = static_cast<float>(std::cos(angl));
            float sine   = static_cast<float>(std::sin(angl));
            float sxc    = size.x  * cosine;
            float syc    = size.y  * cosine;
            float sxs    = size.x  * sine;
            float sys    = size.y  * sine;
            float tx     = -origin.x * sxc - origin.y * sys + position.x;
            float ty     =  origin.x * sxs - origin.y * syc + position.y;

            auto t = robot2D::Transform( sxc, sys, tx,
                                     -sxs, syc, ty,
                                     0.f, 0.f, 1.f);

            return t;
        }

        void draw(robot2D::RenderTarget& target,
                        robot2D::RenderStates states) const {
            //robot2D::Transform transform = getTransform();
            robot2D::Transform transform;
            transform.translate(position);
            transform.scale(size);
            states.transform *= transform;
            states.color = color;
            //states.texture = texture;
            target.draw(states);
        }

        robot2D::Color color;
        robot2D::vec2f position;
        robot2D::vec2f size;
        robot2D::vec2f origin;
        float angle = 0.f;
    };

    BoundingBox() {
        for(auto& wall: m_moveWalls)
            wall.color = m_borderColor;
    }

    void setPosition(const robot2D::vec2f& position) {
        m_aabb.lx = position.x;
        m_aabb.ly = position.y;
    }

    void setSize(const robot2D::vec2f& size) {
        m_aabb.width = size.x;
        m_aabb.height = size.y;
    }

    void setAngle(float angle) {
        m_angle = static_cast<float>(fmod(angle, 360));
    }

    void setBox(const robot2D::FloatRect& rect) {
        m_aabb = rect;
    }

    void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override {

        m_moveWalls[0].position = robot2D::vec2f(m_aabb.lx - m_borderWidth, m_aabb.ly - m_borderWidth);
        m_moveWalls[0].size = robot2D::vec2f(m_aabb.width + m_borderWidth * 2, m_borderWidth);
        m_moveWalls[0].angle = m_angle;

        m_moveWalls[1].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly);
        m_moveWalls[1].size = robot2D::vec2f(m_borderWidth, m_aabb.height);
        m_moveWalls[1].angle = m_angle;

        m_moveWalls[2].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly + m_aabb.height);
        m_moveWalls[2].size = robot2D::vec2f(m_aabb.width, m_borderWidth);
        m_moveWalls[2].angle = m_angle;

        m_moveWalls[3].position = robot2D::vec2f(m_aabb.lx + m_aabb.width, m_aabb.ly);
        m_moveWalls[3].size = robot2D::vec2f(m_borderWidth, m_aabb.height);
        m_moveWalls[3].angle = m_angle;

        for(const auto& wall: m_moveWalls) {
            target.draw(wall);
        }


    }
private:
    float m_borderWidth = 1.f;
    robot2D::Color m_borderColor = robot2D::Color::Red;
    mutable std::array<Wall, 4> m_moveWalls;
    robot2D::FloatRect m_aabb;
    float m_angle;
};


std::vector<Quad> quads;
std::vector<BoundingBox> bbs;

Render2DScene::Render2DScene(robot2D::RenderWindow& window) : Scene(window), m_scene(messageBus) {}


robot2D::FloatRect create4Points(const robot2D::vec2f& point1, const robot2D::vec2f& point2,
                        const robot2D::vec2f& point3, const robot2D::vec2f& point4)  {
    robot2D::vec2f topPoint = point1;
    robot2D::vec2f botPoint = point1;

    std::array<robot2D::vec2f, 4> points = {point1, point2, point3, point4};

    for(int i = 0; i < 3; ++i) {
        if(points[i].x > points[i + 1].x)
            topPoint.x = points[i + 1].x;
        if(points[i].y > points[i + 1].y)
            topPoint.y = points[i + 1].y;
    }

    for(int i = 0; i < 3; ++i) {
        if(points[i].x < points[i + 1].x)
            botPoint.x = points[i + 1].x;
        if(points[i].y < points[i + 1].y)
            botPoint.y = points[i + 1].y;
    }

    return robot2D::FloatRect::create(topPoint, botPoint);
}

robot2D::FloatRect getBoundingBoxRotateCenter(robot2D::FloatRect rect, float degrees) {
    float cx = rect.lx + rect.width / 2;
    float cy = rect.ly + rect.height / 2;
    float wx = rect.width / 2.f;
    float wy = rect.height / 2.f;

    auto convertAngle = [](float degrees) {
        return degrees * (M_PI / 180);
    };

    float sinA = std::sin(convertAngle(degrees));
    float cosA = std::cos(convertAngle(degrees));
    float nH = std::abs(wx * sinA) + std::abs(wy * cosA);
    float nW = std::abs(wx * cosA) + std::abs(wy * sinA);

    return robot2D::FloatRect::create({cx - nW, cy - nH}, {cx + nW,cy + nW});
}

robot2D::vec2f rotatePoint(robot2D::vec2f point, float angle, robot2D::vec2f center_of_rotation) {
    constexpr auto convertAngle = [](float degrees) {
        return degrees * (M_PI / 180);
    };

    float sinus = std::sin(-convertAngle(angle));
    float cosinus = std::cos(-convertAngle(angle));

    robot2D::vec2f tmp;
    point = point - center_of_rotation;
    tmp.x = point.x * cosinus - point.y * sinus;
    tmp.y = point.x * sinus + point.y * cosinus;
    point = tmp + center_of_rotation;
    return point;
}

robot2D::vec2f rotatePoint1(robot2D::vec2f point, float angle, robot2D::vec2f center_of_rotation) {
    constexpr auto convertAngle = [](float degrees) {
        return degrees * (M_PI / 180);
    };

    float sinus = std::sin(convertAngle(angle));
    float cosinus = std::cos(convertAngle(angle));

    robot2D::vec2f tmp;
    point = point - center_of_rotation;
    tmp.x = point.x * cosinus - point.y * sinus;
    tmp.y = point.x * sinus + point.y * cosinus;
    point = tmp + center_of_rotation;
    return point;
}


void Render2DScene::setup() {
    ///// setup Ecs /////
    m_scene.addSystem<RenderSystem>(messageBus);
    m_scene.addSystem<DemoMoveSystem>(messageBus);

    ///// setup Ecs /////

    m_textures.loadFromFile(ResourceID::Logo, texturePath);
    {
        Quad q;
        q.setPosition({400, 400});
        q.setSize({250, 100});
        q.setOrigin({0.5f, 0.5f});
        q.color = robot2D::Color::Cyan;

        float rotateAngle = 40.f;

        q.rotate(rotateAngle);
        quads.emplace_back(q);

        robot2D::FloatRect rect = q.calculateRect();
        float cx = rect.lx + rect.width / 2;
        float cy = rect.ly + rect.height / 2;
        float wx = rect.width / 2.f;
        float wy = rect.height / 2.f;

        auto convertAngle = [](float degrees) {
            return degrees * (M_PI / 180);
        };

        float sinA = std::sin(convertAngle(rotateAngle));
        float cosA = std::cos(convertAngle(rotateAngle));

        robot2D::vec2f p1 = {cx - wx * cosA + wy * sinA , cy - wx * sinA - wy * cosA};
        robot2D::vec2f p2 = {cx + wx * cosA + wy * sinA, cy + wx * sinA - wy * cosA};
        robot2D::vec2f p3 = {cx + wx * cosA - wy * sinA, cy + wx * sinA + wy * cosA};
        robot2D::vec2f p4 = {cx - wx * cosA - wy * sinA, cy - wx * sinA + wy * cosA};

        auto rotateRect = create4Points(
                {cx - wx * cosA + wy * sinA , cy - wx * sinA - wy * cosA},
                {cx + wx * cosA + wy * sinA, cy + wx * sinA - wy * cosA},
                {cx + wx * cosA - wy * sinA, cy + wx * sinA + wy * cosA},
                {cx - wx * cosA - wy * sinA, cy - wx * sinA + wy * cosA}
        );


        float nH = std::abs(wx * sinA) + std::abs(wy * cosA);
        float nW = std::abs(wx * cosA) + std::abs(wy * sinA);


        BoundingBox b;
        //b.setBox(getBoundingBoxRotateCenter(q.calculateRect(), rotateAngle));
        b.setBox(q.getGlobalBounds());
        bbs.emplace_back(b);

        std::array<robot2D::vec2f, 4> pps;
        pps[0] = rotatePoint1(rect.topPoint(),  rotateAngle, rect.centerPoint());
        pps[1] = rotatePoint1(rect.widthPoint(), rotateAngle, rect.centerPoint());
        pps[2] = rotatePoint1(rect.heightPoint(), rotateAngle, rect.centerPoint());
        pps[3] = rotatePoint1(rect.botPoint(), rotateAngle, rect.centerPoint());

        for(const auto& pp: pps)
        {
            Quad q1;
            q1.setPosition(pp);
            q1.setSize({10, 10});
            q1.color = robot2D::Color::Yellow;
            quads.emplace_back(q1);
        }


    }
}



void Render2DScene::handleEvents(const robot2D::Event& event) {
    if(event.type == robot2D::Event::Resized) {
        RB_INFO("New Size = {0} and {1}", event.size.widht, event.size.heigth);
        m_window.resize({static_cast<int>(event.size.widht),
                         static_cast<int>(event.size.heigth)});
        m_window.setView(robot2D::View(robot2D::FloatRect{0, 0,
                                                          static_cast<float>(event.size.widht),
                                                          static_cast<float>(event.size.heigth)
        }));
    }

    if(event.type == robot2D::Event::JoystickConnected) {
        RB_INFO("Joysick Connected ID = {0}", event.joystick.joytickId);
    }

    if(event.type == robot2D::Event::JoystickDisconnected) {
        RB_INFO("Joysick Disconnected ID = {0}", event.joystick.joytickId);
    }

    if(event.type == robot2D::Event::MousePressed) {
        robot2D::vec2f mousePoint = {static_cast<float>(event.mouse.x), static_cast<float>(event.mouse.y)};
        const auto& q = quads[0];


        robot2D::FloatRect rect = q.calculateRect();
        float cx = rect.lx + rect.width / 2;
        float cy = rect.ly + rect.height / 2;

        mousePoint = rotatePoint(mousePoint, q.getRotation(), {cx, cy });

        if(rect.contains(mousePoint))
            quads[0].color = robot2D::Color::White;
    }

    if(event.type == robot2D::Event::MouseReleased) {
        quads[0].color = robot2D::Color::Cyan;
    }

}

void Render2DScene::update(float dt) {
    constexpr float speed = 100.F;

    if(robot2D::isJoystickAvailable(robot2D::JoystickType::One)
        && robot2D::joystickIsGamepad(robot2D::JoystickType::One)) {

        robot2D::JoystickGamepadInput gamepadInput{};
        if(robot2D::getJoystickGamepadInput(robot2D::JoystickType::One, gamepadInput)) {
            auto leftManipulator = gamepadInput.leftManipulatorOffset;

            if((-1.F <= leftManipulator.x && leftManipulator.x <= 1.F)
                    && (-1.F <= leftManipulator.y && leftManipulator.y <= 1.F) ) {
                //q.move(leftManipulator * speed * dt);
            }
        }
    }

    m_scene.update(dt);
}

void Render2DScene::imGuiRender() {

}

robot2D::vec2f getScale(robot2D::vec2f targetSize, robot2D::vec2u originSize) {
    assert(originSize != robot2D::vec2u{});
    if(targetSize.x == originSize.x && targetSize.y == originSize.y)
        return {1.f, 1.f};
    return {targetSize.x / originSize.x, targetSize.y / originSize.y};
}

void Render2DScene::render() {
    m_window.beforeRender();
    m_window.draw(m_scene);
    for(const auto& q: quads)
        m_window.draw(q);
    for(const auto& b: bbs)
        m_window.draw(b);
    m_window.afterRender();
}

robot2D::ecs::Entity Render2DScene::createEntity(const robot2D::vec2f& position) {
    robot2D::ecs::Entity entity = m_scene.createEntity();

    auto& transform = entity.addComponent<TransformComponent>();
    transform.setPosition({position.x, position.y} );

    auto& sprite = entity.addComponent<SpriteComponent>();
    sprite.setTexture(m_textures.get(ResourceID::Logo));
    sprite.layerIndex = 1;

    transform.setSize({100.F, 100.F});

    return entity;
}
