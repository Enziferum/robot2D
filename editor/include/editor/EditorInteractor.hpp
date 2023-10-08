#pragma once

#include <robot2D/Graphics/Rect.hpp>

#include "EditorState.hpp"
#include "UIInteractor.hpp"
#include "Observer.hpp"
#include "EditorState.hpp"

namespace editor {

    class EditorInteractor: public UIInteractor {
    public:
        virtual ~EditorInteractor() = 0;

        virtual void update(float dt) = 0;
        virtual void findSelectEntities(const robot2D::FloatRect& rect) = 0;
        virtual bool hasSelectedEntities() const = 0;
        virtual void createScene() = 0;
        virtual void copyToBuffer() = 0;
        virtual void pasterFromBuffer() = 0;
        virtual void deleteEntity() = 0;
        virtual void undoCommand() = 0;
        virtual void redoCommand() = 0;
        virtual void duplicateEntity(robot2D::vec2f mousePos, robot2D::ecs::Entity entity) = 0;
        virtual void removeSelectedEntities() = 0;
        virtual void addObserver(Observer::Ptr observer) = 0;
        virtual void notifyObservers(std::vector<std::string>&& paths) = 0;
        virtual EditorState getState() const = 0;
        virtual bool saveScene() = 0;
    protected:
    };

} // namespace editor