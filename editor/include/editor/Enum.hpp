#pragma once
#include <type_traits>
#include <limits>
#include <string>
#include <array>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>

namespace editor {

    template<std::size_t Size> struct static_string;

    template<std::size_t Begin, std::size_t End, std::size_t ... Indexes>
    struct make_index_subsequence : make_index_subsequence<Begin, End - 1, End - 1, Indexes ...> {};

    template<std::size_t Pos, std::size_t ... Indexes>
    struct make_index_subsequence<Pos, Pos, Indexes ...> : std::index_sequence<Indexes ...> {};

    namespace helper {
        template<std::size_t Size>
        constexpr std::size_t count(const static_string<Size>& str, char ch, std::size_t index) {
            return index >= Size - 1 ? 0 : (str.m_buffer[index] == ch ? 1 : 0) + count(str, ch, index + 1);
        }
    }


    template<std::size_t Size>
    struct static_string {
        static constexpr std::size_t static_string_npos = std::numeric_limits<std::size_t>::max();

        constexpr std::size_t length() const { return Size - 1; }

        constexpr std::size_t size() const { return Size - 1; }

        constexpr std::size_t begin() const { return 0; }

        constexpr std::size_t end() const { return Size - 1; }

        /// constexpr has if / else in c++17 ??
        constexpr std::size_t find(char ch, std::size_t from = 0, std::size_t nth = 0) const {
            return Size < 2 || from >= Size - 1 ? static_string_npos: m_buffer[from] != ch ?
                                                                      find(ch, from + 1, nth) : nth > 0 ? find(ch, from + 1, nth - 1) : from;
        }

        constexpr std::size_t count( char ch, std::size_t index) const {
            return helper::count(*this, ch, index);
        }

        std::string to_string() {
            return { m_buffer.data() };
        }

        constexpr std::array<const char, Size>& getBuffer() const { return m_buffer; }

        std::array<const char, Size> m_buffer;
    };

    template<std::size_t Size, std::size_t... Indices>
    constexpr static_string<sizeof...(Indices) + 1> make_static_string(const char (&str)[Size],
                                                                       std::index_sequence<Indices...>) {
        return  { str[Indices]... , '\0' };
    }

    template<std::size_t Size>
    constexpr static_string<Size> make_static_string(const char (&str)[Size]) {
        return make_static_string(str, std::make_index_sequence<Size - 1>{});
    }

    template<>
    constexpr static_string<1> make_static_string(const char (&str)[1]) {
        return {'\0'};
    }

    template<std::size_t Size>
    constexpr static_string<Size> make_static_string(const std::array<const char, Size>& buffer) {
        return make_static_string(buffer, std::make_index_sequence<Size - 1>{});
    }

    template<std::size_t Size>
    std::ostream& operator<<(std::ostream& os, const static_string<Size>& str) {
        os << str.data();
        return os;
    }

    class enum_classes_holder {

    public:
        template<typename EnumT, std::size_t InSize>
        static void registerEnum(const std::array<std::string, InSize>& buffer) {
            Value value;
            for (const auto& val : buffer)
                value.push_back(&val);
            Key key = typeid(EnumT);
            m_enumValues[key] = value;
        }

        template<typename EnumT>
        static std::string get_string(int index) {
            Key key = typeid(EnumT);
            const auto& value = m_enumValues.at(key);
            return *value[index];
        }

    private:
        using Key = std::type_index;
        using Value = std::vector<const std::string*>;

        static inline std::unordered_map<Key, Value> m_enumValues;
    };


    template<typename EnumT, std::size_t RetSize, std::size_t InSize>
    constexpr std::array<std::string, RetSize> unpack_enum_string(const static_string<InSize>& enum_str) {
        std::array<std::string, RetSize> result;
        std::size_t arr_index = 0;

        int index = 0;
        while (arr_index < RetSize ) {
            if (arr_index == RetSize - 1) {
                int offset = arr_index == 0 ? 0 : 1;

                auto len = InSize - index - offset;
                std::unique_ptr<char[]> buffer = std::make_unique<char[]>(len + 1);

                for (int i = 0; i < len; ++i)
                    buffer[i] = enum_str.m_buffer[index + i + offset];
                buffer[len] = '\0';
                result[arr_index] = std::string{ buffer.get() };
                break;
            }
            else {
                const std::size_t comma = enum_str.find(',', index);
                {
                    int offset = arr_index == 0 ? 0 : 1;
                    auto len = comma - index - offset;
                    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(len + 1);

                    for (int i = 0; i < len; ++i)
                        buffer[i] = enum_str.m_buffer[index + i + offset];
                    buffer[len] = '\0';
                    result[arr_index] = std::string{ buffer.get() };
                }
                ++arr_index;
                index = comma + 1;
            }
        }

        enum_classes_holder::registerEnum<EnumT, RetSize>(result);

        return result;
    }

#define DECLARE_ENUM(T, ...) \
    enum class T { __VA_ARGS__ };                          \
    struct enum_##T##_holder { \
        static std::string get_string(int index) { return SelfName + "::" + m_arr[index]; }                            \
    private:                      \
        static inline const std::string SelfName = #T;                           \
        static constexpr auto enum_buffer = make_static_string(#__VA_ARGS__);                                    \
        static constexpr std::size_t enum_size = (enum_buffer.count(',', 0) + 1);                     \
        static inline std::array<std::string, enum_size> m_arr         \
            = unpack_enum_string<T, enum_size>(enum_buffer); \
    };

#define ENUM2STR(value) enum_classes_holder::get_string<std::decay_t<decltype(value)>>(static_cast<std::underlying_type_t<std::decay_t<decltype(value)>>>(value))

} // namespace editor