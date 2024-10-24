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

#include <robot2D/imgui/DragDropTarget.hpp>

namespace robot2D {
    void PayloadBuffer::setData(void* buffer, std::size_t bufferSz) {
        m_buffer = buffer;
        m_bufferSz = bufferSz;
    }

    template<>
    std::string PayloadBuffer::unpack() {
        return { static_cast<char*>(m_buffer), m_bufferSz };
    }

    DragDropTarget::DragDropTarget() {
        if(ImGui::BeginDragDropTarget())
            m_beginTarget = true;
    }

    DragDropTarget::DragDropTarget(std::string payloadID): m_payloadID(std::move(payloadID)) {
        if(ImGui::BeginDragDropTarget())
            m_beginTarget = true;
    }

    DragDropTarget::~DragDropTarget() {
        if(m_beginTarget)
            ImGui::EndDragDropTarget();
    }

    PayloadBuffer DragDropTarget::unpackPayload2Buffer(std::string customPayloadID, ImGuiDragDropFlags dragDropFlags) {
        PayloadBuffer payloadBuffer;
        if(!m_beginTarget)
            return payloadBuffer;
        const auto* payload = ImGui::AcceptDragDropPayload(
                customPayloadID.empty() ? m_payloadID.c_str() : customPayloadID.c_str(), dragDropFlags);
        if(!payload || !payload -> IsDataType(customPayloadID.empty() ? m_payloadID.c_str() : customPayloadID.c_str())
           || !payload -> Data)
            return payloadBuffer;
        payloadBuffer.setData(payload -> Data, payload -> DataSize);
        return payloadBuffer;
    }

} // namespace robot2D