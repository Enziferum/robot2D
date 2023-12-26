#pragma once
#include <functional>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/imgui/Gui.hpp>
#include "MessageDispather.hpp"

#include "Editor.hpp"
#include "EditorLogic.hpp"
#include "EditorRouter.hpp"
#include "EditorPresenter.hpp"

namespace editor {

    class IEditorOpener {
    public:
        virtual ~IEditorOpener() = 0;
        virtual void createProject(Project::Ptr project) = 0;
        virtual void loadProject(Project::Ptr project) = 0;
        virtual void closeCurrentProject(std::function<void()>&& resultCallback) = 0;
    };

    class IEditorModule: public IEditorOpener {
    public:
        using Ptr = std::unique_ptr<IEditorModule>;
        virtual ~IEditorModule() = 0;

        virtual void setup(robot2D::RenderWindow* window) = 0;
        virtual void handleEvents(const robot2D::Event& event) = 0;
        virtual void handleMessages(const robot2D::Message& message) = 0;
        virtual void update(float dt) = 0;
        virtual void render() = 0;
        virtual void destroy() = 0;

        virtual void createProject(Project::Ptr project) = 0;
        virtual void loadProject(Project::Ptr project) = 0;
    };

    class EditorModule: public IEditorModule {
    public:
        EditorModule(robot2D::MessageBus& messageBus, MessageDispatcher& messageDispatcher, robot2D::Gui& gui);
        ~EditorModule() override = default;

        void setup(robot2D::RenderWindow* window) override;
        void handleEvents(const robot2D::Event& event) override;
        void handleMessages(const robot2D::Message& message) override;
        void update(float dt) override;
        void render() override;
        void destroy() override;

        ////////////////// IEditorOpener //////////////////
        void createProject(Project::Ptr project) override;
        void loadProject(Project::Ptr project) override;
        virtual void closeCurrentProject(std::function<void()>&& resultCallback) override;
        ////////////////// IEditorOpener //////////////////
    private:
        Editor m_editor;
        EditorRouter m_router;
        EditorPresenter m_presenter;
        EditorLogic m_interactor;
    };

} // namespace editor