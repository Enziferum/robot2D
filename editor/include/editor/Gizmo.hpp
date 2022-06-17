#pragma once

#include <robot2D/Graphics/Rect.hpp>

namespace editor {
    namespace experimental {
        class GizmoManipulator {
        public:
            GizmoManipulator();
            ~GizmoManipulator();

            /// Call after ImGui::Begin();
            void BeginFrame();
            void setRect(const robot2D::FloatRect& rectangle);

            bool isUsing() const;
            void Manipulate();
        private:
            bool m_using;
        };
    }
}