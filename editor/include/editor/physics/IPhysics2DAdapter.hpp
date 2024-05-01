/*********************************************************************
(c) Alex Raag 2023
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

namespace editor {
    using editorEntityList = std::list<SceneEntity>;

    enum class PhysicsAdapterType {
        Box2D
    };

    enum class PhysicsCallbackType: int {
        Enter = 0,
        Exit = 1,
        EnterTrigger = 2,
        ExitTrigger = 3
    };

    struct Physics2DContact {
        UUID entityA;
        UUID entityB;
        PhysicsCallbackType contanctType{PhysicsCallbackType::Enter};
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
        virtual void addRuntime(SceneEntity entity) = 0;
        virtual void registerCallback(PhysicsCallbackType callbackType, PhysicsCallback&& callback) = 0;
    };

    IPhysics2DAdapter::Ptr getPhysics2DAdapter(PhysicsAdapterType);
}