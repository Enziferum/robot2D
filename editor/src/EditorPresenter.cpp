/*********************************************************************
(c) Alex Raag 2026
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

#include <editor/EditorPresenter.hpp>
#include <editor/Editor.hpp>

namespace editor {

    EditorPresenter::EditorPresenter(IEditor* editor): m_editor{editor} {}

    const EditorState& EditorPresenter::getState() const {
        return m_state;
    }

    void EditorPresenter::prepareView() {
        m_editor -> prepareView();
    }

    void EditorPresenter::clearSelectionOnUI() {
        m_editor -> clearSelectionOnUI();
    }

    void EditorPresenter::showPopup(PopupConfiguration* popupConfiguration) {
        m_editor -> showPopup(popupConfiguration);
    }

    bool EditorPresenter::hasAssetChanges() const {
        return m_editor -> hasAssetChanges();
    }

    bool EditorPresenter::saveAssetChanges() {
        return m_editor -> saveAssetChanges();
    }

    void EditorPresenter::discardAssetChanges() {
        m_editor -> discardAssetChanges();
    }

    void EditorPresenter::setMainCameraEntity(SceneEntity entity) {
        m_editor -> setMainCameraEntity(entity);
    }
} // namespace editor
