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
#include <editor/UIManager.hpp>
#include <editor/Components.hpp>
#include <editor/SceneEntity.hpp>

#include "IPanel.hpp"
#include "editor/components/ButtonComponent.hpp"

namespace editor {

    class InspectorPanel: public IPanel {
    public:
        explicit InspectorPanel(MessageDispatcher& messageDispatcher, robot2D::MessageBus& messageBus,
                                PrefabManager& prefabManager,  IUIManager& uiManager);
        InspectorPanel(const InspectorPanel& other) = delete;
        InspectorPanel& operator=(const InspectorPanel& other) = delete;
        InspectorPanel(InspectorPanel&& other) = delete;
        InspectorPanel& operator=(InspectorPanel&& other) = delete;
        ~InspectorPanel() override = default;

        void setInteractor(UIInteractor::Ptr interactor);
        void setSelected(SceneEntity entity);

        void render() override;
        void clearSelection();
    private:
        void drawAssetBase();
        void drawComponentsBase(SceneEntity entity);
        void drawComponents(SceneEntity entity);
        void drawUIComponents(SceneEntity entity);
        
        void drawTransformComponent(SceneEntity, TransformComponent& component);
        void drawCameraComponent(SceneEntity, CameraComponent& component);
        void drawDrawableComponent(SceneEntity, DrawableComponent& component);
        void drawPhysics2DComponent(SceneEntity, Physics2DComponent& component);
        void drawCollider2DComponent(SceneEntity, Collider2DComponent& component);
        void drawTextComponent(SceneEntity, TextComponent& component);
        void drawAnimationComponent(SceneEntity, AnimationComponent& component);
        void drawScriptComponent(SceneEntity, ScriptComponent& component);
        void processScriptComponent(SceneEntity, ScriptComponent& component);

        void drawUIButtonComponent(SceneEntity, ButtonComponent& component);

        void onPrefabAssetSelected(const PrefabAssetPressedMessage& message);
        void onPanelEntityNeedSelect(const PanelEntitySelectedMessage& message);
        void onPanelEntitySelected(const PanelEntitySelectedMessage& message);

        static void onLoadImage(const robot2D::Image& image, SceneEntity entity);
        static void onLoadFont(const robot2D::Font& font, SceneEntity entity);
    private:
        MessageDispatcher& m_messageDispatcher;
        PrefabManager& m_prefabManager;
        robot2D::MessageBus& m_messageBus;
        IUIManager& m_uiManager;
        UIInteractor::Ptr m_interactor{nullptr};
        SceneEntity m_selectedEntity;

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