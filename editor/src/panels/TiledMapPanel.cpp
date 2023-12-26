#include <editor/FileApi.hpp>

#include <robot2D/imgui/Api.hpp>
#include <editor/Macro.hpp>
#include <editor/Messages.hpp>
#include <editor/panels/TiledMapPanel.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/TaskQueue.hpp>
#include <editor/async/ImageLoadTask.hpp>

namespace editor {
    bool tiledButton(const robot2D::Texture& texture, robot2D::vec2i uvCoords, robot2D::vec2f size) {

    }


    TiledMapPanel::TiledMapPanel(robot2D::MessageBus& messageBus): IPanel(typeid(TiledMapPanel)),
    m_messageBus{messageBus}
    {}

    void TiledMapPanel::render() {
        robot2D::WindowOptions windowOptions;
        robot2D::createWindow(windowOptions, BIND_CLASS_FN(TiledMapPanel::windowFunction));
    }

    void TiledMapPanel::windowFunction() {
        // TODO(a.raag): min 1 max 1024 ?
        ImGui::InputInt("TileSize: ", &m_tileSize);
        // TODO(a.raag): retreat down
        if(m_textureMap) {
            tiledMapUI();
        }
        else {
            loadTextureUI();
        }
    }

    void TiledMapPanel::tiledMapUI() {
        auto textureSize = m_textureMap -> getSize();

        int xItems = textureSize.x / m_tileSize;
        int yItems = textureSize.y / m_tileSize;

        for(int i = 0; i < xItems; ++i) {
            for(int y = 0; y < yItems; ++y) {

                if(tiledButton(*m_textureMap, {}, {})) {
                    auto* msg =
                            m_messageBus.postMessage<TileElementSelectedMessage>(MessageID::TileElementSelected);
                    msg -> tileIndex = robot2D::vec2i{i, y};
                }
            }
        }

    }

    void TiledMapPanel::loadTextureUI() {
        if(ImGui::Button("LoadTexture")) {
            /// TODO load
        }

        if (ImGui::BeginDragDropTarget()) {
            // all string types make as Parameters somewhere

//            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
//                const wchar_t *path = (const wchar_t *) payload->Data;
//                std::filesystem::path localPath = std::filesystem::path("assets") / path;
//                auto texturePath = combinePath(m_interactor -> getAssociatedProjectPath(), localPath.string());
//
//                auto manager = ResourceManager::getManager();
//                if (!manager->hasImage(localPath.filename().string())) {
//                    auto queue = TaskQueue::GetQueue();
//                    queue->template addAsyncTask<ImageLoadTask>([this](const ImageLoadTask &task) {
//                        this->onLoadImage(task.getImage(), task.getEntity());
//                    }, texturePath, entity);
//                } else {
//                    auto texture = m_textures.template add(entity.getIndex());
//                    if (texture) {
//                        auto image = manager->getImage(localPath.filename().string());
//                        texture->create(image);
//                        component.setTexture(*texture);
//                    }
//                }
//            }
            ImGui::EndDragDropTarget();
        }
    }


}


