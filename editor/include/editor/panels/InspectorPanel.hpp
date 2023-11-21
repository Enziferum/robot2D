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

#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Graphics/Font.hpp>
#include <robot2D/Util/ResourceHandler.hpp>

#include <robot2D/Ecs/Entity.hpp>
#include <editor/MessageDispather.hpp>
#include <editor/PrefabManager.hpp>
#include <editor/UIInteractor.hpp>
#include <editor/Messages.hpp>

#include "IPanel.hpp"

namespace editor {

    class InspectorPanel: public IPanel {
    public:
        explicit InspectorPanel(MessageDispatcher& messageDispatcher, robot2D::MessageBus& messageBus,
                                PrefabManager& prefabManager);
        ~InspectorPanel() override = default;

        void setInteractor(UIInteractor::Ptr interactor);
        void render() override;
        void clearSelection();
    private:
        void drawComponentsBase(robot2D::ecs::Entity entity, bool isEntity = true);
        void drawComponents(robot2D::ecs::Entity entity, bool isEntity = true);

        void drawAssetBase();

        void onPrefabAssetSelected(const PrefabAssetPressedMessage& message);

        void onLoadImage(const robot2D::Image& image, robot2D::ecs::Entity entity);
        void onLoadFont(const robot2D::Font& font, robot2D::ecs::Entity entity);
    private:
        MessageDispatcher& m_messageDispatcher;
        PrefabManager& m_prefabManager;
        robot2D::MessageBus& m_messageBus;

        robot2D::ecs::Entity m_selectedEntity;

        UIInteractor::Ptr m_interactor{nullptr};

        robot2D::ResourceHandler<robot2D::Texture, robot2D::ecs::EntityID> m_textures;
        robot2D::ResourceHandler<robot2D::Font, robot2D::ecs::EntityID> m_fonts;


        enum class InspectType {
            EditorEntity,
            AssetPrefab,
            AssetScene,
            AssetImage,
            AssetFont
        } m_inspectType = InspectType::EditorEntity;

        bool m_prefabHasModification{false};
    };
}