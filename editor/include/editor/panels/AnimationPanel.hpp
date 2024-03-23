/*********************************************************************
(c) Alex Raag 2024
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

#include <imgui/imgui.h>
#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Core/MessageBus.hpp>

#include <editor/Animation.hpp>
#include <editor/Observer.hpp>
#include <editor/UIInteractor.hpp>
#include <editor/MessageDispather.hpp>
#include <editor/Messages.hpp>

#include "editor/async/AnimationTextureSliceTask.hpp"
#include "IPanel.hpp"

namespace editor {

    struct UIKeyFrame {
        UIKeyFrame() = default;
        UIKeyFrame(const UIKeyFrame& other) = default;
        UIKeyFrame& operator=(const UIKeyFrame& other) = default;
        UIKeyFrame(UIKeyFrame&& other) = default;
        UIKeyFrame& operator=(UIKeyFrame&& other) = default;
        ~UIKeyFrame() = default;

        bool isClicked(const robot2D::vec2f& mousePos) const noexcept {
            return m_bb.Contains(mousePos);
        }

        void updatePosition(ImVec2 pos) {
            m_bb = ImRect(pos, { pos.x + m_size.x, pos.y + m_size.y });
        }

        void draw(ImDrawList* drawList) {
            drawList -> AddQuadFilled(m_bb.GetTL(), m_bb.GetBL(), m_bb.GetBR(), m_bb.GetTR(),
                                      m_moving ? m_selectColor : m_color);
        }

        int& getCurrentFrame() noexcept { return m_currentFrame; }
        void setMoving(bool flag) { m_moving = flag; }
    private:
        int m_currentFrame { 0 };

        ImRect m_bb;
        ImVec2 m_size{ 10, 10 };

        ImColor m_color = 0xFFFFFFFF; // white
        ImColor m_selectColor = 0xA02A2AFF; // red

        bool m_moving{false};
        bool needDel{false};
    };

    struct Cursor {
        Cursor();
        Cursor(const Cursor& other) = delete;
        Cursor& operator=(const Cursor& other) = delete;
        Cursor(Cursor&& other) = delete;
        Cursor& operator=(Cursor&& other) = delete;
        ~Cursor() = default;

        void reset();

        void nextKeyFrame();
        void prevKeyFrame();

        void setPosition(const robot2D::vec2f& position);
        void moveX(float offset);
        void draw(ImDrawList* drawList, robot2D::vec2f length, int drawFrame);
    private:
        robot2D::vec2f m_pos;
    };


    class AnimationPanel final: public IPanel {
    public:
        AnimationPanel(robot2D::MessageBus& messageBus,
                       MessageDispatcher& messageDispatcher, UIInteractor::Ptr interactor);
        AnimationPanel(const AnimationPanel& other) = delete;
        AnimationPanel& operator=(const AnimationPanel& other) = delete;
        AnimationPanel(AnimationPanel&& other) = delete;
        AnimationPanel& operator=(AnimationPanel&& other) = delete;
        ~AnimationPanel() override = default;

        void render() override;
    private:
        void renderHeader(ImGuiWindow* window, ImDrawList* draw_list);
        void renderSequencer_old();
        void renderSequencer();
        void renderScrollBar(ImGuiIO& io);
        void processDragDrop();
        void renderSlicePreview();


        void onAnimationSlice(const AnimationTextureSliceTask& task);
        void onSelectEntity(const PanelEntitySelectedMessage& message);

        void drawLine(ImDrawList* draw_list, int i, int regionHeight);
        void drawLineContent(ImDrawList* draw_list, int i);
    private:
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        UIInteractor::Ptr m_interactor;

        std::list<UIKeyFrame> m_keyFrames;
        Cursor m_cursor;
        UIKeyFrame* m_currentKeyFrame = nullptr;
        Animation* m_currentAnimation{nullptr};
        bool m_needShowSlicePreview{false};
        std::string m_loadTexturePath;
        robot2D::ecs::Entity m_animationEntity;

        enum class PlayMode: int {
            Play = 0,
            Stop
        } m_mode = PlayMode::Play;

        bool m_MovingScrollBar{false};
        bool m_MovingCurrentFrame{false};

        int m_currentFrame{ 0 };
        int m_firstFrame{ 0 };
        int m_frameMin = 0;
        int m_frameMax = 1000;
        int m_modFrameCount = 10;
        int m_frameStep = 1;
        int m_halfModFrameCount{ 0 };
        int m_sequenceCount = 1;

        int m_frameCount;
        int m_visibleFrameCount;
        int m_firstFrameUsed{ 0 };
        int m_panningViewFrame;

        size_t m_customHeight{0};

        float m_height{ 300 };
        float m_framePixelWidth{ 10.f };
        float m_framePixelWidthTarget{ 10.f };
        float m_barWidthRatio;

        ImVec2 m_canvas_pos;
        ImVec2 m_canvas_size;
        ImVec2 panningViewSource;
        ImVec2 m_scrollBarSize;
        ImVec2 m_childFrameSize;
        ImVec2 m_childFramePos;
        ImVec2 m_contentMin;
        ImVec2 m_contentMax;
    };
}