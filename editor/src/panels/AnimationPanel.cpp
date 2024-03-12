#include <vector>
#include <robot2D/imgui/Api.hpp>
#include <imgui/imgui_internal.h>
#include <robot2D/Util/Logger.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Ecs/EntityManager.hpp>


#include <editor/panels/AnimationPanel.hpp>
#include <editor/AnimationManager.hpp>
#include <editor/FiledialogAdapter.hpp>
#include <editor/FileApi.hpp>
#include <editor/Macro.hpp>

#include <editor/LocalResourceManager.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/TaskQueue.hpp>
#include <editor/Messages.hpp>
#include <editor/Components.hpp>
#include <editor/DragDropIDS.hpp>
#include <editor/async/ImageLoadTask.hpp>


namespace editor {
#ifndef IMGUI_DEFINE_MATH_OPERATORS
    static ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
        return ImVec2(a.x + b.x, a.y + b.y);
    }
#endif

    bool renderButton(std::string text, ImDrawList* draw_list, ImVec2 pos, float* outButtonWidth = nullptr) {
        ImGuiIO& io = ImGui::GetIO();
        constexpr float textOffset = 2.f;
        auto textSize = ImGui::CalcTextSize(text.c_str());
        ImRect btnRect(pos, ImVec2(pos.x + textSize.x + textOffset, pos.y + textSize.y + textOffset));
        bool overBtn = btnRect.Contains(io.MousePos);
        bool containedClick = overBtn && btnRect.Contains(io.MouseClickedPos[0]);
        bool clickedBtn = containedClick && io.MouseReleased[0];
        int btnColor = overBtn ? 0xAAEAFFAA : 0x77A3B2AA;
        if (containedClick && io.MouseDownDuration[0] > 0)
            btnRect.Expand(2.0f);

        draw_list -> AddRect(btnRect.Min, btnRect.Max, btnColor, 4);
        ImVec2 tpos = ImVec2{btnRect.Min.x + 1, btnRect.Min.y + 1};
        draw_list -> AddText(tpos, 0xFFFFFFFF, text.c_str());
        if(outButtonWidth)
            *outButtonWidth = btnRect.GetWidth();

        return clickedBtn;
    }

    bool renderInvisibleButton(ImRect bb, bool* hovered) {
        ImGuiIO& io = ImGui::GetIO();
        bool overBtn = bb.Contains(io.MousePos);
        if(hovered)
            *hovered = overBtn;
        bool containedClick = overBtn && bb.Contains(io.MouseClickedPos[0]);
        bool clickedBtn = containedClick && io.MouseReleased[0];

        return clickedBtn;
    }

    /// TODO(a.raag) ??
    void renderNavHighlight(ImRect bb) {
//        ImGuiContext& g = *GImGui;
//        if (id != g.NavId)
//            return;
//        if (g.NavDisableHighlight && !(flags & ImGuiNavHighlightFlags_AlwaysDraw))
//            return;
//        ImGuiWindow* window = g.CurrentWindow;
//        if (window->DC.NavHideHighlightOneFrame)
//            return;
//
//        float rounding = (flags & ImGuiNavHighlightFlags_NoRounding) ? 0.0f : g.Style.FrameRounding;
//        ImRect display_rect = bb;
//        display_rect.ClipWith(window->ClipRect);
//        if (flags & ImGuiNavHighlightFlags_TypeDefault)
//        {
//            const float THICKNESS = 2.0f;
//            const float DISTANCE = 3.0f + THICKNESS * 0.5f;
//            display_rect.Expand(ImVec2(DISTANCE, DISTANCE));
//            bool fully_visible = window->ClipRect.Contains(display_rect);
//            if (!fully_visible)
//                window->DrawList->PushClipRect(display_rect.Min, display_rect.Max);
//            window->DrawList->AddRect(display_rect.Min + ImVec2(THICKNESS * 0.5f, THICKNESS * 0.5f), display_rect.Max - ImVec2(THICKNESS * 0.5f, THICKNESS * 0.5f), GetColorU32(ImGuiCol_NavHighlight), rounding, 0, THICKNESS);
//            if (!fully_visible)
//                window->DrawList->PopClipRect();
//        }
//        if (flags & ImGuiNavHighlightFlags_TypeThin)
//        {
//            window->DrawList->AddRect(display_rect.Min, display_rect.Max, GetColorU32(ImGuiCol_NavHighlight), rounding, 0, 1.0f);
//        }
    }


    bool renderCombo(ImGuiWindow* window, ImDrawList* drawList, ImVec2 pos, ImVec2 size,
                     std::string preview, bool* popupHovered) {
        ImGuiContext& g = *GImGui;
        static const char* label = "##AnimPanelComboPopup";

        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window -> GetID (label);

        const float arrow_size = ImGui::GetFrameHeight();
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
        const float w = 100.F; // TODO(a.raag) correct

        const ImRect bb(pos, pos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
        const ImRect total_bb(bb.Min, bb.Max + ImVec2(label_size.x > 0.0f
                        ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

        //bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        bool hovered;
        bool pressed = renderInvisibleButton(bb, &hovered);
        if(*popupHovered)
            *popupHovered = hovered;

        const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
        bool popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
        if (pressed && !popup_open)
        {
            ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
            popup_open = true;
        }

        // Render shape
        const ImU32 frame_col = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        const float value_x2 = ImMax(bb.Min.x, bb.Max.x - arrow_size);
        renderNavHighlight(bb);

        drawList -> AddRectFilled(bb.Min, ImVec2(value_x2, bb.Max.y), frame_col, style.FrameRounding,
                                  ImDrawFlags_RoundCornersLeft);
        {
            ImU32 bg_col = ImGui::GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
            drawList -> AddRectFilled(ImVec2(value_x2, bb.Min.y), bb.Max, bg_col,
                                      style.FrameRounding, (w <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);
            if (value_x2 + arrow_size - style.FramePadding.x <= bb.Max.x)
                ImGui::RenderArrow(drawList,
                                   ImVec2(value_x2 + style.FramePadding.y, bb.Min.y + style.FramePadding.y),
                                   text_col, ImGuiDir_Down, 1.0f);
        }
        ImGui::RenderFrameBorder(bb.Min, bb.Max, style.FrameRounding);


        // Render preview and label
        if (!preview.empty())
        {
            ImGui::RenderTextClipped(
                    bb.Min + style.FramePadding, ImVec2(value_x2, bb.Max.y),
                    preview.c_str(), NULL, NULL);
        }

        if (!popup_open)
            return false;

        return ImGui::BeginComboPopup(popup_id, bb, 0);
    }


    bool renderInputInt(ImDrawList* drawList, std::string label, int* value, ImVec2 pos) {
        const char* format = "%d";
        /// TODO(a.raag): InputText ?
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        auto textSize = ImGui::CalcTextSize(label.c_str());
        const ImRect labelBB(pos, pos + ImVec2(textSize.x, textSize.y + style.FramePadding.y * 2.0f));
        const float label_x2 = ImMax(labelBB.Min.x, labelBB.Max.x);

        ImGui::RenderTextClipped(
                labelBB.Min + style.FramePadding, ImVec2(label_x2, labelBB.Max.y),
                label.c_str(), NULL, NULL);

        return false;
    }


    ////////////////////////////// UIKeyFrame //////////////////////////////////////////


    ////////////////////////////// UIKeyFrame //////////////////////////////////////////



    ////////////////////////////// Cursor //////////////////////////////////////////

    AnimationPanel::Cursor::Cursor() {

    }

    void AnimationPanel::Cursor::reset() {

    }

    void AnimationPanel::Cursor::nextKeyFrame() {

    }

    void AnimationPanel::Cursor::prevKeyFrame() {

    }

    void AnimationPanel::Cursor::draw(ImDrawList* drawList) {

    }


    ////////////////////////////// Cursor //////////////////////////////////////////


    AnimationPanel::AnimationPanel(robot2D::MessageBus& messageBus,
                                   MessageDispatcher& messageDispatcher, UIInteractor::Ptr interactor):
        IPanel(typeid(AnimationPanel)),
        m_messageBus{messageBus},
        m_messageDispatcher{messageDispatcher},
        m_interactor{interactor}{


        m_messageDispatcher.onMessage<PanelEntitySelectedMessage>(
                MessageID::PanelEntityNeedSelect,BIND_CLASS_FN(onSelectEntity));

    }

    void AnimationPanel::render() {
        if(ImGui::Begin("Animation")) {
            renderSequencer();
            processDragDrop();
            ImGui::End();
        }

        if(m_needShowSlicePreview)
            renderSlicePreview();
    }

    void AnimationPanel::drawLine(ImDrawList* draw_list, int i, int regionHeight) {
        bool baseIndex = ((i % m_modFrameCount) == 0) || (i == m_frameMax || i == m_frameMin);
        bool halfIndex = (i % m_halfModFrameCount) == 0;
        int px = (int)m_canvas_pos.x + int(i * m_framePixelWidth) + m_legendWidth - int(m_firstFrameUsed * m_framePixelWidth);
        int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
        int tiretEnd = baseIndex ? regionHeight : m_ItemHeight;

        if (px <= (m_canvas_size.x + m_canvas_pos.x) && px >= (m_canvas_pos.x + m_legendWidth))
        {
            draw_list -> AddLine(ImVec2((float)px, m_canvas_pos.y + (float)tiretStart),
                                 ImVec2((float)px, m_canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

            draw_list -> AddLine(ImVec2((float)px, m_canvas_pos.y + (float)m_ItemHeight),
                                 ImVec2((float)px, m_canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
        }

        if (baseIndex && px > (m_canvas_pos.x + m_legendWidth))
        {
            char tmps[512];
            ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
            draw_list -> AddText(ImVec2((float)px + 3.f, m_canvas_pos.y), 0xFFBBBBBB, tmps);
        }

    }


    void AnimationPanel::drawLineContent(ImDrawList* draw_list, int i, int) {
        int px = (int)m_canvas_pos.x + int(i * m_framePixelWidth) + m_legendWidth - int(m_firstFrameUsed * m_framePixelWidth);
        int tiretStart = int(contentMin.y);
        int tiretEnd = int(contentMax.y);

        if (px <= (m_canvas_size.x + m_canvas_pos.x) && px >= (m_canvas_pos.x + m_legendWidth))
        {
            //draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

            draw_list -> AddLine(ImVec2(float(px), float(tiretStart)),
                                 ImVec2(float(px), float(tiretEnd)), 0x30606060, 1);
        }
    }


    void AnimationPanel::renderHeader(ImGuiWindow* window, ImDrawList* draw_list) {
        static const char* textButtonImages[7] = {"<<|", "<|", ">", "||", "|>", "|>>", "Save"};
        constexpr float xOffset = 3.f;
        float startButtonOffsetX = 10;
        float outButtonWidth = 0;
        static int item_current_idx = 0;
        m_popupOpened = false;

        std::string previewAnimation = m_currentAnimation ? m_currentAnimation -> name : "No Animation";
        auto* localManager = LocalResourceManager::getManager();

        if(renderButton(textButtonImages[0], draw_list,
                        {m_canvas_pos.x + startButtonOffsetX, m_canvas_pos.y + 2}, &outButtonWidth)) {
            /// TODO(a.raag): move to first frame
        }

        if(outButtonWidth != 0)
            startButtonOffsetX += (outButtonWidth + 2);

        if(renderButton(textButtonImages[1], draw_list,
                        {m_canvas_pos.x + startButtonOffsetX, m_canvas_pos.y + 2}, &outButtonWidth)) {
            /// TODO(a.raag): move cursor to prev Frame
            m_cursor.prevKeyFrame();
        }

        if(outButtonWidth != 0)
            startButtonOffsetX += (outButtonWidth + 2);

        std::string playButton = m_mode == PlayMode::Play ? textButtonImages[2] : textButtonImages[3];
        if(renderButton(playButton, draw_list,
                        {m_canvas_pos.x + startButtonOffsetX, m_canvas_pos.y + 2}, &outButtonWidth)) {
            if(m_interactor -> isRunning() || !m_currentAnimation)
                return;

            auto* msg =
                    m_messageBus.postMessage<AnimationPlayMessage>(MessageID::AnimationPlay);
            msg -> entity = m_animationEntity;
            msg -> mode = static_cast<AnimationPlayMessage::Mode>(static_cast<int>(m_mode));

            if(m_mode == PlayMode::Play) {
                m_mode = PlayMode::Stop;
                m_currentAnimation -> isLooped = true;
            }
            else if(m_mode == PlayMode::Stop) {
                m_mode = PlayMode::Play;
                m_currentAnimation -> isLooped = false;
            }

            /// Start Move cursor from start to end looping
        }

        if(outButtonWidth != 0)
            startButtonOffsetX += (outButtonWidth + 2);

        if(renderButton(textButtonImages[4], draw_list,
                        {m_canvas_pos.x + startButtonOffsetX, m_canvas_pos.y + 2}, &outButtonWidth)) {
            /// TODO(a.raag): move cursor to next Frame
            m_cursor.nextKeyFrame();
        }

        if(outButtonWidth != 0)
            startButtonOffsetX += (outButtonWidth + 2);

        if(renderButton(textButtonImages[5], draw_list,
                        {m_canvas_pos.x + startButtonOffsetX, m_canvas_pos.y + 2}, &outButtonWidth)) {
            /// TODO(a.raag): move cursor to last frame
        }

        if(outButtonWidth != 0)
            startButtonOffsetX += (outButtonWidth + 2);

        if(renderButton(textButtonImages[6], draw_list,
                        {m_canvas_pos.x + startButtonOffsetX, m_canvas_pos.y + 2}, &outButtonWidth)) {
            if(!m_currentAnimation)
                return;

//            if(!manager -> saveAnimation(m_currentAnimation)) {
//                RB_EDITOR_ERROR("AnimationPanel: Cant' Save Animation");
//            }
        }

        draw_list -> PushClipRect(m_childFramePos, m_childFramePos + m_childFrameSize,
                                  true);

        if(renderCombo(window, draw_list, {contentMin.x + xOffset,contentMin.y + 1},
                       {}, previewAnimation, &m_popupHovered)) {
            if(localManager -> hasAnimations(m_animationEntity.getComponent<IDComponent>().ID)) {
                for(auto& animation:
                        localManager -> getAnimations(m_animationEntity.getComponent<IDComponent>().ID)) {
                    const bool is_selected = (m_currentAnimation -> name == animation.name);
                    if (ImGui::Selectable(animation.name.c_str(), is_selected)) {
                        m_currentAnimation = &animation;
                        m_animationEntity.getComponent<AnimationComponent>().setAnimation(m_currentAnimation);
                        m_keyFrames.clear();

                        int frameOffset = 0;
                        for(auto& frame: m_currentAnimation -> frames) {
                            UIKeyFrame keyFrame;
                            keyFrame.currentFrame = frameOffset;
                            frameOffset += m_currentAnimation -> framesPerSecond;
                            m_keyFrames.emplace_back(keyFrame);
                        }
                    }

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }
            if(ImGui::Button("Add New")) {
                auto* dialogAdapter = FiledialogAdapter::get();

                if(!m_animationEntity || m_animationEntity.destroyed()) {
                    ///// TODO(a.raag): no-op
                }

                std::string path;
                if(!dialogAdapter -> saveFile(path, "Save Animation")) {
                    RB_EDITOR_ERROR("AnimationPanel: Cant' get Save Path");
                    return;
                }

                std::string name = getFileName(path);
                if(name.empty()) {
                    RB_EDITOR_ERROR("AnimationPanel: Can't get filename on creation");
                    /// errror
                }

                if(!m_animationEntity.hasComponent<AnimationComponent>()) {
                    if(!m_animationEntity.hasComponent<AnimationComponent>())
                        m_animationEntity.addComponent<AnimationComponent>();
                    m_animationEntity.addComponent<AnimationComponent>();
                }

                auto& animationComponent = m_animationEntity.getComponent<AnimationComponent>();
                m_currentAnimation = nullptr;
                m_currentAnimation = localManager -> addAnimation(m_animationEntity.getComponent<IDComponent>().ID);
                if(!m_currentAnimation) {
                    /// TODO(a.raag): Error
                }

                m_currentAnimation -> name = name;
                m_currentAnimation -> filePath = path;
                m_animationEntity.getComponent<AnimationComponent>().setAnimation(m_currentAnimation);
                m_keyFrames.clear();

                //m_popupOpened = false;
            }

            m_popupOpened = true;
            ImGui::EndPopup();
        }

        int value = 0;
        constexpr float hardCodeComboWidth = 102.f;
        if(renderInputInt(draw_list, "Frames", &value,
                          {contentMin.x + xOffset + hardCodeComboWidth,contentMin.y + 1})) {

        }

        m_customHeight = 0;
    }

    void AnimationPanel::renderSequencer() {
        ImGui::BeginGroup();

        ImGuiIO& io = ImGui::GetIO();
        auto window = ImGui::GetCurrentWindow();
        int cx = (int)(io.MousePos.x);
        int cy = (int)(io.MousePos.y);

        robot2D::vec2i mousePos = {
                static_cast<int>(io.MousePos.x),
                static_cast<int>(io.MousePos.y),
        };

        static int movingPos = -1;
        static int movingPart = -1;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        m_canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
        m_canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
        m_firstFrameUsed = m_firstFrame;
        m_customHeight = 0;
        m_height = m_canvas_size.y;

        int controlHeight = m_sequenceCount * m_ItemHeight;
        for (int i = 0; i < m_sequenceCount; i++)
            controlHeight += int(m_height);
        m_frameCount = ImMax(m_frameMax - m_frameMin, 1);

        struct CustomDraw
        {
            int index;
            ImRect customRect;
            ImRect legendRect;
            ImRect clippingRect;
            ImRect legendClippingRect;
        };

        ImVector<CustomDraw> customDraws;
        ImVector<CustomDraw> compactCustomDraws;

        // zoom in/out
        m_visibleFrameCount = (int)floorf((m_canvas_size.x - m_legendWidth) / m_framePixelWidth);
        m_barWidthRatio = ImMin(m_visibleFrameCount / (float)m_frameCount, 1.f);


        ImRect regionRect(m_canvas_pos, m_canvas_pos + m_canvas_size);


        if (ImGui::IsWindowFocused() && io.KeyAlt && io.MouseDown[2])
        {
            if (!m_panningView)
            {
                panningViewSource = io.MousePos;
                m_panningView = true;
                m_panningViewFrame = m_firstFrame;
            }
            m_firstFrame = m_panningViewFrame - int((io.MousePos.x - panningViewSource.x) / m_framePixelWidth);
            m_firstFrame = ImClamp(m_firstFrame, m_frameMin, m_frameMax - m_visibleFrameCount);
        }
        if (m_panningView && !io.MouseDown[2])
        {
            m_panningView = false;
        }

        m_framePixelWidthTarget = ImClamp(m_framePixelWidthTarget, 0.1f, 50.f);
        m_framePixelWidth = ImLerp(m_framePixelWidth, m_framePixelWidthTarget, 0.33f);

        m_frameCount = m_frameMax -m_frameMin;
        if (m_visibleFrameCount >= m_frameCount)
            m_firstFrame = m_frameMin;



        bool hasScrollBar(true);
        /*
        int framesPixelWidth = int(frameCount * framePixelWidth);
        if ((framesPixelWidth + legendWidth) >= canvas_size.x)
        {
            hasScrollBar = true;
        }
        */
        // test scroll area
        ImVec2 headerSize(m_canvas_size.x, (float)m_ItemHeight);
        m_scrollBarSize = ImVec2(m_canvas_size.x, 14.f);
        ImGui::InvisibleButton("topBar", headerSize);
        draw_list -> AddRectFilled(m_canvas_pos, m_canvas_pos + headerSize, 0xFFFF0000, 0);
        m_childFramePos = ImGui::GetCursorScreenPos();
        m_childFrameSize = ImVec2(m_canvas_size.x, m_canvas_size.y - 8.f
                                    - headerSize.y - (hasScrollBar ? m_scrollBarSize.y : 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
        ImGui::BeginChildFrame(889, m_childFrameSize);
        m_focused = ImGui::IsWindowFocused();
        m_visible = ImGui::IsWindowHovered();

        ImGui::InvisibleButton("contentBar", ImVec2(m_canvas_size.x, float(controlHeight)));
        contentMin = ImGui::GetItemRectMin();
        contentMax = ImGui::GetItemRectMax();
        const ImRect contentRect(contentMin, contentMax);
        const float contentHeight = contentMax.y - contentMin.y;

        // full background
        draw_list -> AddRectFilled(m_canvas_pos, m_canvas_pos + m_canvas_size, 0xFF242424, 0);

        // current frame top
        ImRect topRect(ImVec2(m_canvas_pos.x + m_legendWidth, m_canvas_pos.y),
                       ImVec2(m_canvas_pos.x + m_canvas_size.x, m_canvas_pos.y + m_ItemHeight));

        if (!m_MovingCurrentFrame && !m_MovingScrollBar && m_movingEntry == -1
            && m_currentFrame >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0])
        {
            m_MovingCurrentFrame = true;
        }
        if (m_MovingCurrentFrame)
        {
            if (m_frameCount)
            {
                m_currentFrame = (int)((io.MousePos.x - topRect.Min.x) / m_framePixelWidth) + m_firstFrameUsed;
                if (m_currentFrame < m_frameMin)
                    m_currentFrame = m_frameMin;
                if (m_currentFrame >= m_frameMax)
                    m_currentFrame = m_frameMax;
            }
            if (!io.MouseDown[0])
                m_MovingCurrentFrame = false;
        }

        draw_list -> AddRectFilled(m_canvas_pos, ImVec2(m_canvas_size.x + m_canvas_pos.x,
                                                        m_canvas_pos.y + m_ItemHeight), 0xFF3D3837, 0);

        /// render Frame's TimeLine
        {
            m_modFrameCount = 10;
            m_frameStep = 1;

            while ((m_modFrameCount * m_framePixelWidth) < 150)
            {
                m_modFrameCount *= 2;
                m_frameStep *= 2;
            };
            m_halfModFrameCount = m_modFrameCount / 2;

            for (int i = m_frameMin; i <= m_frameMax; i += m_frameStep)
            {
                drawLine(draw_list, i, m_ItemHeight);
            }
            drawLine(draw_list, m_frameMin, m_ItemHeight);
            drawLine(draw_list, m_frameMax, m_ItemHeight);
        }


        renderHeader(window, draw_list);


        // slots background
        for (int i = 0; i < m_sequenceCount; i++)
        {
            unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

            size_t localCustomHeight = m_height;
            ImVec2 pos = ImVec2(contentMin.x + m_legendWidth, contentMin.y + m_ItemHeight * i + 1 + m_customHeight);
            ImVec2 sz = ImVec2(m_canvas_size.x + m_canvas_pos.x, pos.y + m_ItemHeight - 1 + localCustomHeight);
            if (!(m_popupOpened || m_popupHovered) && cy >= pos.y
                && cy < pos.y + (m_ItemHeight + localCustomHeight) && m_movingEntry == -1
                && cx > contentMin.x && cx < contentMin.x + m_canvas_size.x)
            {
                col += 0x80201008;
                pos.x -= m_legendWidth;
            }
            draw_list -> AddRectFilled(pos, sz, col, 0);
            m_customHeight += localCustomHeight;
        }

        draw_list -> PushClipRect(m_childFramePos + ImVec2(float(m_legendWidth), 0.f),
                                  m_childFramePos + m_childFrameSize, true);

        // vertical frame lines in content area
        for (int i = m_frameMin; i <= m_frameMax; i += m_frameStep)
        {
            drawLineContent(draw_list, i, int(contentHeight));
        }
        drawLineContent(draw_list, m_frameMin, int(contentHeight));
        drawLineContent(draw_list, m_frameMax, int(contentHeight));


        /// selection
        bool selected = m_selectedEntry;
        if (selected)
        {
            m_customHeight = 0;
            for (int i = 0; i < m_selectedEntry; i++)
                m_customHeight += m_height;
            draw_list -> AddRectFilled(ImVec2(contentMin.x, contentMin.y + m_ItemHeight * m_selectedEntry + m_customHeight),
                                     ImVec2(contentMin.x + m_canvas_size.x, contentMin.y + m_ItemHeight * (m_selectedEntry + 1) + m_customHeight), 0x801080FF, 1.f);
        }

        /// slots
        m_customHeight = 0;
        for (int i = 0; i < m_sequenceCount; i++)
        {
            int* start, * end;
            unsigned int color;
            /// TODO(a.raag): Get
            Get(i, &start, &end, NULL, &color);

            size_t localCustomHeight = m_height;

            ImVec2 pos = ImVec2(contentMin.x + m_legendWidth - m_firstFrameUsed * m_framePixelWidth,
                                contentMin.y + m_ItemHeight * i + 1 + m_customHeight);
            ImVec2 slotP1(pos.x + *start * m_framePixelWidth, pos.y + 2);
            ImVec2 slotP2(pos.x + *end * m_framePixelWidth + m_framePixelWidth, pos.y + m_ItemHeight - 2);
            ImVec2 slotP3(pos.x + *end * m_framePixelWidth + m_framePixelWidth, pos.y + m_ItemHeight - 2 + localCustomHeight);
            unsigned int slotColor = color | 0xFF000000;
            unsigned int slotColorHalf = (color & 0xFFFFFF) | 0x40000000;

            if (slotP1.x <= (m_canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + m_legendWidth))
            {
                draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
                draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
            }
            if (ImRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
            {
               // sequence -> DoubleClick(i);
            }

            // Ensure grabbable handles
            const float max_handle_width = slotP2.x - slotP1.x / 3.0f;
            const float min_handle_width = ImMin(10.0f, max_handle_width);
            const float handle_width = ImClamp(m_framePixelWidth / 2.0f, min_handle_width, max_handle_width);
            ImRect rects[3] = { ImRect(slotP1, ImVec2(slotP1.x + handle_width, slotP2.y))
                    , ImRect(ImVec2(slotP2.x - handle_width, slotP1.y), slotP2)
                    , ImRect(slotP1, slotP2) };

            const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, slotColor + (selected ? 0 : 0x202020) };
            if (m_movingEntry == -1 )// TODOFOCUS && backgroundRect.Contains(io.MousePos))
            {
                for (int j = 2; j >= 0; j--)
                {
                    ImRect& rc = rects[j];
                    if (!rc.Contains(io.MousePos))
                        continue;
                    draw_list -> AddRectFilled(rc.Min, rc.Max, quadColor[j], 2);
                }

                for (int j = 0; j < 3; j++)
                {
                    ImRect& rc = rects[j];
                    if (!rc.Contains(io.MousePos))
                        continue;
                    if (!ImRect(m_childFramePos, m_childFramePos + m_childFrameSize).Contains(io.MousePos))
                        continue;
                    if (ImGui::IsMouseClicked(0) && !m_MovingScrollBar && !m_MovingCurrentFrame)
                    {
                        m_movingEntry = i;
                        movingPos = cx;
                        movingPart = j + 1;
                        // TODO(a.raag) BeginEdit
                        // sequence -> BeginEdit(movingEntry);
                        break;
                    }
                }
            }

            // custom draw
            if (localCustomHeight > 0)
            {
                ImVec2 rp(m_canvas_pos.x, contentMin.y + m_ItemHeight * i + 1 + m_customHeight);
                ImRect customRect(rp + ImVec2(m_legendWidth - (m_firstFrameUsed - m_frameMin - 0.5f) * m_framePixelWidth, float(m_ItemHeight)),
                                  rp + ImVec2(m_legendWidth + (m_frameMax - m_firstFrameUsed - 0.5f + 2.f) * m_framePixelWidth, float(localCustomHeight + m_ItemHeight)));
                ImRect clippingRect(rp + ImVec2(float(m_legendWidth), float(m_ItemHeight)), rp + ImVec2(m_canvas_size.x, float(localCustomHeight + m_ItemHeight)));

                ImRect legendRect(rp + ImVec2(0.f, float(m_ItemHeight)), rp + ImVec2(float(m_legendWidth), float(localCustomHeight)));
                ImRect legendClippingRect(m_canvas_pos + ImVec2(0.f, float(m_ItemHeight)), m_canvas_pos + ImVec2(float(m_legendWidth), float(localCustomHeight + m_ItemHeight)));
                customDraws.push_back({ i, customRect, legendRect, clippingRect, legendClippingRect });
            }
            else
            {
                ImVec2 rp(m_canvas_pos.x, contentMin.y + m_ItemHeight * i + m_customHeight);
                ImRect customRect(rp + ImVec2(m_legendWidth - (m_firstFrameUsed -m_frameMin - 0.5f) * m_framePixelWidth, float(0.f)),
                                  rp + ImVec2(m_legendWidth + (m_frameMax - m_firstFrameUsed - 0.5f + 2.f) * m_framePixelWidth, float(m_ItemHeight)));
                ImRect clippingRect(rp + ImVec2(float(m_legendWidth), float(0.f)), rp + ImVec2(m_canvas_size.x, float(m_ItemHeight)));

                compactCustomDraws.push_back({ i, customRect, ImRect(), clippingRect, ImRect() });
            }
            m_customHeight += localCustomHeight;
        }

        /// Key Frames
        for(auto& keyFrame: m_keyFrames) {

            ImVec2 framePos;
            framePos.x = contentMin.x + m_legendWidth +
                    (float)keyFrame.currentFrame * m_framePixelWidth - 5 - 3;
            framePos.y = m_canvas_pos.y + 80;

            keyFrame.updatePosition(framePos);
            if(keyFrame.bb.Contains(io.MousePos) && ImGui::IsMouseClicked(0)) {
                movingPos = cx;
                m_currentKeyFrame = &keyFrame;
            }

            keyFrame.draw(draw_list);
        }

        /// Move Key Frame
        if(m_currentKeyFrame) {
            ImGui::SetNextFrameWantCaptureMouse(true);
            int diffFrame = int((cx - movingPos) / m_framePixelWidth);
            if(std::abs(diffFrame) > 0) {
                m_currentKeyFrame -> m_moving = true;
                ImVec2 framePos;
                m_currentKeyFrame -> currentFrame += diffFrame;
                framePos.x = contentMin.x + m_legendWidth +
                             (float)m_currentKeyFrame -> currentFrame * m_framePixelWidth - 5 - 3;
                framePos.y = m_canvas_pos.y + 80;

                m_currentKeyFrame -> updatePosition(framePos);
                movingPos += int(diffFrame * m_framePixelWidth);
            }
            if (!io.MouseDown[0])
            {
                m_currentKeyFrame -> m_moving = false;
                m_currentKeyFrame = nullptr;
            }
        }

        if(ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
            if(m_currentKeyFrame)
                m_currentKeyFrame -> needDel = true;
        }


        /// moving
        if (m_movingEntry >= 0)
        {
            ImGui::SetNextFrameWantCaptureMouse(true);

            int diffFrame = int((cx - movingPos) / m_framePixelWidth);
            if (std::abs(diffFrame) > 0)
            {
                int* start, *end;
                // TODO(a.raag): GetMethod ??
                Get(m_movingEntry, &start, &end, NULL, NULL);

                m_selectedEntry = m_movingEntry;
                int& l = *start;
                int& r = *end;
                if (movingPart & 1)
                    l += diffFrame;
                if (movingPart & 2)
                    r += diffFrame;
                if (l < 0)
                {
                    if (movingPart & 2)
                        r -= l;
                    l = 0;
                }
                if (movingPart & 1 && l > r)
                    l = r;
                if (movingPart & 2 && r < l)
                    r = l;
                movingPos += int(diffFrame * m_framePixelWidth);
            }
            if (!io.MouseDown[0])
            {
                // single select
                if (!diffFrame && movingPart)
                {
                    m_selectedEntry = m_movingEntry;
                    // ret = true;
                }

                m_movingEntry = -1;
                // sequence -> EndEdit();
            }
        }



        /// Red Cursor
        if (m_currentFrame >= m_firstFrame && m_currentFrame <= m_frameMax)
        {
            static const float cursorWidth = 4.f;
            float cursorOffset = contentMin.x + m_legendWidth +
                    (m_currentFrame - m_firstFrameUsed) * m_framePixelWidth + m_framePixelWidth / 2 - cursorWidth * 0.5f;
            draw_list -> AddLine(ImVec2(cursorOffset, m_canvas_pos.y),
                                 ImVec2(cursorOffset, contentMax.y), 0xA02A2AFF, cursorWidth);
            char tmps[512];
            ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", m_currentFrame);
            draw_list -> AddText(ImVec2(cursorOffset + 10, m_canvas_pos.y + 2), 0xFF2A2AFF, tmps);
        }

        draw_list -> PopClipRect();
        draw_list -> PopClipRect();

        ImGui::EndChildFrame();
        ImGui::PopStyleColor();

        renderScrollBar(io);
        ImGui::EndGroup();

        if (regionRect.Contains(io.MousePos))
        {
            bool overCustomDraw = false;
            for (auto& custom : customDraws)
            {
                if (custom.customRect.Contains(io.MousePos))
                {
                    overCustomDraw = true;
                }
            }
            if (overCustomDraw)
            {
            }
            else
            {
#if 0
                frameOverCursor = *firstFrame + (int)(visibleFrameCount * ((io.MousePos.x - (float)legendWidth - canvas_pos.x) / (canvas_size.x - legendWidth)));
            //frameOverCursor = max(min(*firstFrame - visibleFrameCount / 2, frameCount - visibleFrameCount), 0);

            /**firstFrame -= frameOverCursor;
            *firstFrame *= framePixelWidthTarget / framePixelWidth;
            *firstFrame += frameOverCursor;*/
            if (io.MouseWheel < -FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 1.1f);
               framePixelWidthTarget *= 0.9f;
               *firstFrame += frameOverCursor;
            }

            if (io.MouseWheel > FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 0.9f);
               framePixelWidthTarget *= 1.1f;
               *firstFrame += frameOverCursor;
            }
#endif
            }
        }


        if(m_currentKeyFrame) {
            if(!m_currentKeyFrame -> needDel)
                return;
        }

    }

    void AnimationPanel::renderScrollBar(ImGuiIO& io) {
        auto draw_list = ImGui::GetWindowDrawList();
        const float barWidthInPixels = m_barWidthRatio * (m_canvas_size.x - m_legendWidth);

        ImGui::InvisibleButton("scrollBar", m_scrollBarSize);
        ImVec2 scrollBarMin = ImGui::GetItemRectMin();
        ImVec2 scrollBarMax = ImGui::GetItemRectMax();

        // ratio = number of frames visible in control / number to total frames



        float startFrameOffset = ((float)(m_firstFrameUsed - m_frameMin) / (float)m_frameCount) * (m_canvas_size.x - m_legendWidth);
        ImVec2 scrollBarA(scrollBarMin.x + m_legendWidth, scrollBarMin.y - 2);
        ImVec2 scrollBarB(scrollBarMin.x + m_canvas_size.x, scrollBarMax.y - 1);
        draw_list -> AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

        ImRect scrollBarRect(scrollBarA, scrollBarB);
        bool inScrollBar = scrollBarRect.Contains(io.MousePos);

        draw_list -> AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);


        ImVec2 scrollBarC(scrollBarMin.x + m_legendWidth + startFrameOffset, scrollBarMin.y);
        ImVec2 scrollBarD(scrollBarMin.x + m_legendWidth + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
        draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || m_MovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

        ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
        ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

        bool onLeft = barHandleLeft.Contains(io.MousePos);
        bool onRight = barHandleRight.Contains(io.MousePos);

        static bool sizingRBar = false;
        static bool sizingLBar = false;

        draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, (onLeft || sizingLBar) ? 0xFFAAAAAA : 0xFF666666, 6);
        draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, (onRight || sizingRBar) ? 0xFFAAAAAA : 0xFF666666, 6);

        ImRect scrollBarThumb(scrollBarC, scrollBarD);
        static const float MinBarWidth = 44.f;
        if (sizingRBar)
        {
            if (!io.MouseDown[0])
            {
                sizingRBar = false;
            }
            else
            {
                float barNewWidth = ImMax(barWidthInPixels + io.MouseDelta.x, MinBarWidth);
                float barRatio = barNewWidth / barWidthInPixels;
                m_framePixelWidthTarget = m_framePixelWidth = m_framePixelWidth / barRatio;
                int newVisibleFrameCount = int((m_canvas_size.x - m_legendWidth) / m_framePixelWidthTarget);
                int lastFrame = m_firstFrame + newVisibleFrameCount;
                if (lastFrame > m_frameMax)
                {
                    m_framePixelWidthTarget = m_framePixelWidth = (m_canvas_size.x - m_legendWidth) / float(m_frameMax - m_firstFrame);
                }
            }
        }
        else if (sizingLBar)
        {
            if (!io.MouseDown[0])
            {
                sizingLBar = false;
            }
            else
            {
                if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
                {
                    float barNewWidth = ImMax(barWidthInPixels - io.MouseDelta.x, MinBarWidth);
                    float barRatio = barNewWidth / barWidthInPixels;
                    float previousFramePixelWidthTarget = m_framePixelWidthTarget;
                    m_framePixelWidthTarget = m_framePixelWidth = m_framePixelWidth / barRatio;
                    int newVisibleFrameCount = int(m_visibleFrameCount / barRatio);
                    int newFirstFrame = m_firstFrame + newVisibleFrameCount - m_visibleFrameCount;
                    newFirstFrame = ImClamp(newFirstFrame, m_frameMin, ImMax(m_frameMax - m_visibleFrameCount, m_frameMin));
                    if (newFirstFrame == m_firstFrame)
                    {
                        m_framePixelWidth = m_framePixelWidthTarget = previousFramePixelWidthTarget;
                    }
                    else
                    {
                        m_firstFrame = newFirstFrame;
                    }
                }
            }
        }
        else
        {
            if (m_MovingScrollBar)
            {
                if (!io.MouseDown[0])
                {
                    m_MovingScrollBar = false;
                }
                else
                {
                    float framesPerPixelInBar = barWidthInPixels / (float)m_visibleFrameCount;
                    m_firstFrame = int((io.MousePos.x - panningViewSource.x) / framesPerPixelInBar) - m_panningViewFrame;
                    m_firstFrame = ImClamp(m_firstFrame, m_frameMin, ImMax(m_frameMax - m_visibleFrameCount, m_frameMin));
                }
            }
            else
            {
                if (scrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0)  && !m_MovingCurrentFrame && m_movingEntry == -1)
                {
                    m_MovingScrollBar = true;
                    panningViewSource = io.MousePos;
                    m_panningViewFrame = -m_frameMin;
                }
                if (!sizingRBar && onRight && ImGui::IsMouseClicked(0))
                    sizingRBar = true;
                if (!sizingLBar && onLeft && ImGui::IsMouseClicked(0))
                    sizingLBar = true;

            }
        }
    }


    void AnimationPanel::processDragDrop() {
        if(ImGui::BeginDragDropTarget()) {
            auto* payload = ImGui::AcceptDragDropPayload(contentItemID);
            if(!payload || !payload -> IsDataType(contentItemID)) {
                ImGui::EndDragDropTarget();
                return;
            }

            const char* data = static_cast<const char*>(payload -> Data);
            if(!data) {
                ImGui::EndDragDropTarget();
                return;
            }

            std::string payloadString = std::string(data, payload -> DataSize);
            const wchar_t* rawPath = (const wchar_t *) payloadString.c_str();
            std::filesystem::path prefabPath = std::filesystem::path("assets") / rawPath;
            auto realPath = combinePath(m_interactor -> getAssociatedProjectPath(),
                                        prefabPath.string());

            //// TODO(a.raag): check if .jpg is normal works + add map of valid extensions
            if(getFileExtension(realPath) != ".png")
                return;

            m_loadTexturePath = realPath;
            m_needShowSlicePreview = true;

            ImGui::EndDragDropTarget();
        }
    }

    void AnimationPanel::renderSlicePreview() {

        if(!ImGui::Begin("AnimationSlice", &m_needShowSlicePreview, ImGuiWindowFlags_NoDocking)) {
            ImGui::End();
            return;
        }

        auto drawList = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImGuiIO& io = ImGui::GetIO();

        static ImU32 defaultColor = IM_COL32(0, 255, 0, 255);
        static ImU32 selectColor = 0xA02A2AFF;
        static std::vector<AnimationFrame> gluingFrames;

        if(m_currentAnimation -> texture) {
            auto textureSize = m_currentAnimation -> texture -> getSize();
            ImVec2 tSize{static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)};

            //// Draw Texture
            {
                ImTextureID textureID = (ImTextureID) nullptr;
                std::memcpy(&textureID, &m_currentAnimation -> texture -> getID(), sizeof(unsigned int));
                ImRect bb(canvas_p0, canvas_p0 + tSize);
                drawList -> AddImage(textureID, bb.Min, bb.Max, {0,0}, {1, 1});
            }

            int textNumber = 1;
            for(auto& frame: m_currentAnimation -> frames) {
                ImVec2 tPos{frame.frame.lx + frame.frame.width / 2.f, frame.frame.ly - 20.f};
                drawList -> AddText({canvas_p0 + tPos},
                                    IM_COL32(255, 255, 255, 255), std::to_string(textNumber).c_str());

                ImVec2 lT = canvas_p0 + ImVec2{static_cast<float>(frame.frame.lx - 1), static_cast<float>(frame.frame.ly - 1)};
                ImVec2 rB = canvas_p0 + ImVec2{static_cast<float>(frame.frame.lx + frame.frame.width + 2),
                                               static_cast<float>(frame.frame.ly + frame.frame.height + 2)};

                /// TODO(a.raag): change color to green default and glue are red //
                auto found = std::find_if(gluingFrames.begin(), gluingFrames.end(),
                                          [&frame](const auto& item) {
                    return frame.frameIndex == item.frameIndex;
                });
                drawList -> AddRect(lT, rB, found == gluingFrames.end() ? defaultColor : selectColor);

                ImRect bb = {lT, rB};

                if(bb.Contains(io.MousePos) && bb.Contains(io.MouseClickedPos[0]) && found == gluingFrames.end())
                    gluingFrames.emplace_back(frame);

                ++textNumber;
            }

            auto offsetPos = canvas_p0 + tSize;
            float outWidth = 0.f;
            if(gluingFrames.size() > 1) {
                if(renderButton("Glue Frames", drawList, offsetPos, &outWidth)) {
                    robot2D::vec2i minPoint = { gluingFrames[0].frame.lx, gluingFrames[0].frame.ly };
                    robot2D::vec2i maxPoint = { gluingFrames[0].frame.widthPoint().x,
                                                gluingFrames[0].frame.heightPoint().y};

                    for(auto& item: gluingFrames) {
                        auto& frame = item.frame;
                        if(minPoint.x > frame.lx)
                            minPoint.x = frame.lx;
                        if(minPoint.y > frame.ly)
                            minPoint.y = frame.ly;

                        if(maxPoint.x < frame.widthPoint().x)
                            maxPoint.x = frame.widthPoint().x;
                        if(maxPoint.y < frame.heightPoint().y)
                            maxPoint.y = frame.heightPoint().y;

                    }
                    robot2D::IntRect gluedRect = robot2D::IntRect::create(minPoint, maxPoint);
                    AnimationFrame animationFrame;
                    animationFrame.addRect(gluedRect);
                     m_currentAnimation -> replaceFrames(gluingFrames, animationFrame);
                     gluingFrames.clear();
                }
            }

            if(gluingFrames.empty()) {
                /// TODO(a.raag): confirm button if only slice done
                if(renderButton("Confirm", drawList, offsetPos + ImVec2(outWidth, 0.f))) {
                    m_currentAnimation -> setFrameIndices();
                    int frameOffset = 0;
                    for(auto& frame: m_currentAnimation -> frames) {
                        UIKeyFrame keyFrame;
                        keyFrame.currentFrame = frameOffset;
                        frameOffset += m_currentAnimation -> framesPerSecond;
                        m_keyFrames.emplace_back(keyFrame);
                    }
                }
            }
        }
        else {
            float outWidth;
            auto offsetPos = canvas_p0 + ImVec2{100, 100};

            if(renderButton("Slice", drawList, offsetPos, &outWidth)) {
                auto name = getFileName(m_loadTexturePath);
                TaskQueue::GetQueue() -> addAsyncTask<AnimationTextureSliceTask>(
                        BIND_CLASS_FN(onAnimationSlice), name,  m_loadTexturePath
                );
            }
        }

        ImGui::End();
    }

    void AnimationPanel::onAnimationSlice(const AnimationTextureSliceTask& task) {
        auto& frames = task.getRects();
        constexpr int framesPerSecond = 12;

        auto* localManager = LocalResourceManager::getManager();
        auto* manager = ResourceManager::getManager();

        auto* texture = localManager -> addTexture(task.getFileName());
        if(!texture) {
            RB_EDITOR_ERROR("AnimationPanel::onAnimationSlice: Can't create texture");
            return;
        }

        texture -> create(manager -> getImage(task.getFileName()));

        if(!m_currentAnimation) {
            RB_EDITOR_ERROR("AnimationPanel::onAnimationSlice: No Animation");
        }

        m_currentAnimation -> texture = texture;
        m_currentAnimation -> texturePath = cutPath(task.getFilePath(), "assets");
        m_keyFrames.clear();
        m_currentKeyFrame = nullptr;

        int frameOffset = 0;

        for(auto& frame: frames)
            m_currentAnimation -> addFrame(frame);

        m_currentAnimation -> framesPerSecond = 10.f; // HardCode only for Test
    }

    void AnimationPanel::onSelectEntity(const editor::PanelEntitySelectedMessage& message) {
        m_animationEntity = message.entity;
        m_currentAnimation = nullptr;
        m_keyFrames.clear();

        if(!m_animationEntity.hasComponent<AnimationComponent>())
            return;

        m_currentAnimation = m_animationEntity.getComponent<AnimationComponent>().getAnimation();
        int frameOffset = 0;
        for(auto& frame: m_currentAnimation -> frames) {
            UIKeyFrame keyFrame;
            keyFrame.currentFrame = frameOffset;
            frameOffset += m_currentAnimation -> framesPerSecond;
            m_keyFrames.emplace_back(keyFrame);
        }
    }


}