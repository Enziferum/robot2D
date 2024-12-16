#pragma once

namespace robot2D::ecs {

    class class_id {
    public:
        class_id(const char* name);

        // TODO(a.raag): can't be default due to atomic inside
        class_id(const class_id& other) = default;
        class_id& operator=(const class_id& other) = default;
        class_id(class_id&& other) = default;
        class_id& operator=(class_id&& other) = default;

        ~class_id() = default;

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

} // namespace robot2D::ecs