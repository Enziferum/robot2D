#pragma once

namespace editor {

    class class_id {
    public:
        class_id(const char* name);

        int getIndex() const { return m_index; }
        const char* const getName() const { return m_name; }
        bool operator == (const class_id& another) const noexcept
        {
            return m_index == another.m_index;
        }
    private:
        const char* const m_name;
        int m_index;
    };

#define DECLARE_COMPONENT_ID() \
    static const class_id& id() noexcept;

}