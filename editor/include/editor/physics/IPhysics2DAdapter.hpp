/*********************************************************************
(c) Alex Raag 2025
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

#pragma once
#include <memory>
#include <list>
#include <functional>
#include <editor/SceneEntity.hpp>
#include <editor/Uuid.hpp>
#include <editor/ClassID.hpp>

namespace editor {
    using editorEntityList = std::list<SceneEntity>;

    enum class PhysicsAdapterType {
        Box2D
    };

    enum class PhysicsEventType : uint8_t {
        Enter = 0, Stay=1, Exit=2,
        EnterTrigger, ExitTrigger, StayTrigger
    };

#pragma pack(push, 1)
    struct PhysicsContactPoint2D {
        DECLARE_COMPONENT_ID();

        float px, py;
        float sep;
    };

    struct PhysicsContact2D {
        DECLARE_COMPONENT_ID();

        uint64_t entityA, entityB;      // твои ID
        uint64_t fixtureA, fixtureB;    // опционально: ID/хэндлы фикстур
        PhysicsEventType type;

        // Геометрия/динамика
        float nx, ny;                    // нормаль (как в Box2D: от A к B)
        PhysicsContactPoint2D points[2]; // до 2 точек
        uint8_t pointCount;              // фактическое число точек
        float normalImpulse;             // суммарный нормальный импульс за PostSolve
        float tangentImpulse;            // суммарный тангенциальный импульс

        // Флаги
        bool isSensorA, isSensorB;
        uint16_t categoryA, categoryB;   // categoryBits
        uint16_t maskA, maskB;           // maskBits
    };
#pragma pack(pop)

    using PhysicsCallback = std::function<void(const PhysicsContact2D&, UUID self, UUID other)>;

    class IPhysics2DAdapter {
    public:
        using Ptr = std::unique_ptr<IPhysics2DAdapter>;
    public:
        virtual ~IPhysics2DAdapter() = 0;
        virtual void update(float dt) = 0;

        virtual void start(editorEntityList& entityList) = 0;
        virtual void stop() = 0;
        virtual void addRuntime(SceneEntity entity) = 0;
        virtual void registerCallback(PhysicsCallback&& callback) = 0;

        static_assert(sizeof(PhysicsContactPoint2D) == 12, "ContactPoint size");
        static_assert(sizeof(PhysicsContact2D) == 84, "Contact2D size");
    };

    IPhysics2DAdapter::Ptr getPhysics2DAdapter(PhysicsAdapterType);
}