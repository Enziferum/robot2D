#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <robot2D/Ecs/Entity.hpp>

namespace editor {
    using editorEntityList = std::vector<robot2D::ecs::Entity>;

    enum class PhysicsAdapterType {
        Box2D
    };

    enum class PhysicsCallbackType {
        Enter,
        Stay,
        Exit
    };

    struct Physics2DContact {
        std::uint32_t entityA;
        std::uint32_t entityB;
    };

    using PhysicsCallback = std::function<void(const Physics2DContact&)>;

    class IPhysics2DAdapter {
    public:
        using Ptr = std::unique_ptr<IPhysics2DAdapter>;
    public:
        virtual ~IPhysics2DAdapter() = 0;
        virtual void update(float dt) = 0;

        virtual void start(editorEntityList& entityList) = 0;
        virtual void stop() = 0;
        virtual void registerCallback(PhysicsCallbackType callbackType, PhysicsCallback&& callback) = 0;
    };

    IPhysics2DAdapter::Ptr getPhysics2DAdapter(PhysicsAdapterType);
}