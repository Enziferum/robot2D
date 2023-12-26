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


    class AnimationPanel: public IPanel {
    public:
        AnimationPanel(robot2D::MessageBus& messageBus,
                       MessageDispatcher& messageDispatcher, UIInteractor::Ptr interactor);
        ~AnimationPanel() override = default;

        void render() override;
    private:
        void renderHeader(ImGuiWindow* window, ImDrawList* draw_list);
        void renderSequencer();
        void renderScrollBar(ImGuiIO& io);
        void processDragDrop();
        void renderSlicePreview();


        void onAnimationSlice(const AnimationTextureSliceTask& task);
        void onSelectEntity(const PanelEntitySelectedMessage& message);

        void drawLine(ImDrawList* draw_list, int i, int regionHeight);
        void drawLineContent(ImDrawList* draw_list, int i, int /*regionHeight*/);

        void Get(int index, int** start, int** end, int* type, unsigned int* color)
        {

            if (color)
                *color = 0xFFAA8080; // same color for everyone, return color based on type
            if (start)
                *start = &mFrameStart;
            if (end)
                *end = &mFrameEnd;
        }
    private:
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        UIInteractor::Ptr m_interactor;


        struct UIKeyFrame {

            UIKeyFrame() {

            }

            void updatePosition(ImVec2 pos) {
                bb = ImRect(pos, {pos.x + size.x, pos.y + size.y});
            }

            void draw(ImDrawList* drawList) {
                drawList -> AddQuadFilled(bb.GetTL(), bb.GetBL(), bb.GetBR(), bb.GetTR(),
                                          m_moving ? m_selectColor : m_color);
            }

            int currentFrame = 20;

            ImRect bb;
            ImVec2 size{10, 10};
            bool m_moving{false};
            ImColor m_color = 0xFFFFFFFF; // white
            ImColor m_selectColor = 0xA02A2AFF; // red
            bool needDel{false};
        };

        struct Cursor {
            Cursor();

            void reset();

            void nextKeyFrame();
            void prevKeyFrame();

            void draw(ImDrawList* drawList);
        } m_cursor;



        std::list<UIKeyFrame> m_keyFrames;
        UIKeyFrame* m_currentKeyFrame = nullptr;
        Animation* m_currentAnimation{nullptr};
        bool m_needShowSlicePreview{false};
        std::string m_loadTexturePath;
        robot2D::ecs::Entity m_animationEntity;

        bool m_popupOpened{false};
        bool m_popupHovered{false};

        enum class PlayMode: int {
            Play = 0,
            Stop
        } m_mode = PlayMode::Play;


        float m_height{300};
        bool m_focused{false};
        bool m_visible{false};

        int mFrameStart = 0;
        int mFrameEnd = 100;

        int m_currentFrame{0};
        int m_firstFrame{0};
        int m_selectedEntry{-1};

        int m_frameMin = 0;
        int m_frameMax = 1000;

        size_t m_customHeight{0};
        int m_ItemHeight{20};
        bool m_MovingScrollBar{false};
        bool m_MovingCurrentFrame{false};

        int m_legendWidth{200};
        float m_framePixelWidth{10.f};
        float m_framePixelWidthTarget{10.f};
        int m_sequenceCount = 1;

        int m_modFrameCount = 10;
        int m_frameStep = 1;
        int m_halfModFrameCount{0};

        ImVec2 m_canvas_pos;
        ImVec2 m_canvas_size;

        int m_frameCount;
        int m_visibleFrameCount;
        float m_barWidthRatio;

        int m_firstFrameUsed;

        bool m_panningView = false;
        ImVec2 panningViewSource;
        int m_panningViewFrame;
        int m_movingEntry = -1;
        ImVec2 m_scrollBarSize;
        ImVec2 m_childFrameSize;
        ImVec2 m_childFramePos;
        ImVec2 contentMin;
        ImVec2 contentMax;
    };

}