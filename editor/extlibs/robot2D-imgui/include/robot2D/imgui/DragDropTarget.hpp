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
#include <string>
#include <utility>
#include <imgui/imgui.h>

namespace robot2D {

    struct PayloadBuffer {
        PayloadBuffer() = default;
        PayloadBuffer(const PayloadBuffer& other) = default;
        PayloadBuffer& operator=(const PayloadBuffer& other) = default;
        PayloadBuffer(PayloadBuffer&& other) = default;
        PayloadBuffer& operator=(PayloadBuffer&& other) = default;
        ~PayloadBuffer() = default;

        explicit operator bool() const noexcept {
            return m_buffer && m_bufferSz;
        }

        void setData(void* buffer, std::size_t bufferSz);
        template<typename T>
        T unpack();
    private:
        void* m_buffer { nullptr };
        std::size_t m_bufferSz { 0 };
    };

    /// \brief ImGui DragDropTarget simple RAII wrapper. Must be scoped due to ImGui rules.
    class DragDropTarget {
    public:
        explicit DragDropTarget(std::string  payloadID);
        DragDropTarget();
        DragDropTarget(const DragDropTarget& other) = delete;
        DragDropTarget& operator=(const DragDropTarget& other) = delete;
        DragDropTarget(DragDropTarget&& other) = delete;
        DragDropTarget& operator=(DragDropTarget&& other) = delete;
        ~DragDropTarget();

        /// \brief Use when cant easy cast from void* from ImGuiPayload to your Type
        template<typename T>
        T* unpackPayload(std::string customPayloadID = "", ImGuiDragDropFlags dragDropFlags = ImGuiDragDropFlags_None);

        /// \brief Use when can't easy cast from void* from ImGuiPayload to your Type and need size for example std::string
        PayloadBuffer unpackPayload2Buffer(std::string customPayloadID = "",
                                           ImGuiDragDropFlags dragDropFlags = ImGuiDragDropFlags_None);
    private:
        std::string m_payloadID{""};
        bool m_beginTarget { false };
    };

    template<typename T>
    inline T* DragDropTarget::unpackPayload(std::string customPayloadID, ImGuiDragDropFlags dragDropFlags) {
        if(!m_beginTarget)
            return nullptr;
        const auto* payload = ImGui::AcceptDragDropPayload(
                        customPayloadID.empty() ? m_payloadID.c_str() : customPayloadID.c_str(), dragDropFlags);
        if(!payload || !payload -> IsDataType(customPayloadID.empty() ? m_payloadID.c_str() : customPayloadID.c_str())
                || !payload -> Data)
            return nullptr;
        /// TODO(a.raag): think about DragDropPayloadUnpacker::unpack(void* buffer, std::size_t size)
        return static_cast<T*>(payload -> Data);
    }
} // namespace robot2D