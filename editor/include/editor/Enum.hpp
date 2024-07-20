#pragma once
#include <type_traits>
#include <limits>
#include <string>
#include <array>

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


/*
        template<size_t End> constexpr auto prefix() const {
            return substring<0, End>();
        }
        template<size_t Begin> constexpr auto suffix() const {
            return substring<Begin, Size - 1>();
        }

        template<size_t Index> constexpr auto split() const {
            return std::make_pair(prefix<Index>(), suffix<Index + 1>());
        }
*/

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

    template<std::size_t RetSize, std::size_t InSize>
    constexpr std::array<std::string, RetSize> unpack_enum_string(const static_string<InSize>& enum_str) {
        std::array<std::string, RetSize> result;
        std::size_t arr_index = 0;

        int index = 0;
        while(arr_index < RetSize - 1) {
            const std::size_t comma = enum_str.find(',', index);
            {
                char buff[comma];
                int offset = arr_index == 0 ? 0 : 1;
                for(int i = 0; i < comma - index - offset ; ++i)
                    buff[i] = enum_str.m_buffer[index + i + offset];
                result[arr_index] = buff;
            }
            ++arr_index;
            index = comma + 1;
            if(arr_index == RetSize - 1) {
                char buff[comma];
                int offset = arr_index == 0 ? 0 : 1;
                for(int i = 0; i < InSize - index - offset ; ++i)
                    buff[i] = enum_str.m_buffer[index + i + offset];
                result[arr_index] = buff;
            }
        }

        return result;
    }

#define DECLARE_ENUM(T, Values...) \
    enum class T { Values };                          \
    struct enum_##T##_holder { \
        static std::string get_string(int index) { return SelfName + "::" + m_arr[index]; }                            \
    private:                      \
        static inline const std::string SelfName = #T;                           \
        static constexpr auto enum_buffer = make_static_string(#Values);                                    \
        static constexpr std::size_t enum_size = (enum_buffer.count(',', 0) + 1);                     \
        static inline std::array<std::string, enum_size> m_arr         \
            = unpack_enum_string<enum_size>(enum_buffer); \
    };

#define ENUM2STR(T, value) enum_##T##_holder::get_string(static_cast<std::underlying_type_t<T>>(value))

} // namespace editor