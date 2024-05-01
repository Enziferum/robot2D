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

#include <robot2D/Ecs/System.hpp>
#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Graphics/FrameBuffer.hpp>

#include <robot2D/Graphics/Drawable.hpp>

namespace editor {

    class Scene;
    class RenderSystem: public robot2D::ecs::System, public robot2D::Drawable {
    public:
        RenderSystem(robot2D::MessageBus& messageBus);
        RenderSystem(const RenderSystem& other) = delete;
        RenderSystem& operator=(const RenderSystem& other) = delete;
        RenderSystem(RenderSystem&& other) = delete;
        RenderSystem& operator=(RenderSystem&& other) = delete;
        ~RenderSystem() override = default;

        void setBefore(robot2D::ecs::Entity source, robot2D::ecs::Entity target);
        void setFrameBuffer(robot2D::FrameBuffer::Ptr frameBuffer) { m_frameBuffer = frameBuffer; }
        void setScene(Scene* scene);
        void setRuntimeFlag(bool flag) { m_runtimeFlag = flag; }
        void update(float dt) override;
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;
    private:
        bool m_needUpdateZBuffer;
        bool m_runtimeFlag{false};

        Scene* m_activeScene{nullptr};
        robot2D::FrameBuffer::Ptr m_frameBuffer{nullptr};

        enum class ReorderDeleteType {
            First, Last
        };

        using Iterator = std::vector<robot2D::ecs::Entity>::iterator;
        using InsertItem = std::tuple<Iterator, robot2D::ecs::Entity, ReorderDeleteType>;

        std::vector<InsertItem> m_insertItems;
        robot2D::View m_cameraView;
    };

}