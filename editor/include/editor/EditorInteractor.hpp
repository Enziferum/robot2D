#pragma once

#include <robot2D/Graphics/Rect.hpp>
#include <robot2D/Core/Event.hpp>

#include "EditorState.hpp"
#include "UIInteractor.hpp"
#include "Observer.hpp"
#include "EditorState.hpp"
#include "EditorCamera.hpp"

namespace editor {

    class EditorInteractor: public UIInteractor {
    public:
        virtual ~EditorInteractor() = 0;

        virtual void handleEventsRuntime(const robot2D::Event& event) = 0;
        virtual void update(float dt) = 0;
        virtual void findSelectEntities(const robot2D::FloatRect& rect) = 0;
        virtual bool hasSelectedEntities() const = 0;
        virtual void copyToBuffer() = 0;
        virtual void pasterFromBuffer() = 0;
        virtual void undoCommand() = 0;
        virtual void redoCommand() = 0;
        virtual void duplicateEntity(robot2D::vec2f mousePos) = 0;
        virtual void removeSelectedEntities() = 0;
        virtual void addObserver(Observer::Ptr observer) = 0;
        virtual void notifyObservers(std::vector<std::string>&& paths) = 0;
        virtual EditorState getState() const = 0;
        virtual bool saveScene() = 0;
        virtual robot2D::vec2f getMainCameraPosition() const = 0;
        virtual void setMainCamera(SceneEntity cameraEntity) = 0;
        virtual void setEditorCamera(IEditorCamera::Ptr editorCamera) = 0;
    protected:
    };

} // namespace editor