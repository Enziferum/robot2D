#pragma once
#include <tuple>
#include <type_traits>
#include <string>
#include <iostream>
#include <filesystem>

namespace editor {
    namespace helper {
        namespace __impl {
# if __cplusplus >= 201703L

            template<typename MaybeString>
            inline constexpr bool is_string = std::is_convertible_v<MaybeString, std::string>;

            template<>
            inline constexpr bool is_string<std::string_view> = true;
#elif __cplusplus == 201402L
            template<typename MaybeString>
            constexpr bool is_string = std::is_convertible<MaybeString, std::string>::value;
#endif
        }
        template<typename T>
        struct formatter {};
# if __cplusplus >= 201703L
        template<typename T>
        auto to_string(const T& t) -> decltype(formatter<T>::to_string(t)) {
            return formatter<T>::to_string(t);
        }

        template<typename Numeric>
        auto to_string(Numeric numeric) -> decltype(std::to_string(numeric)) {
            return std::to_string(numeric);
        }

        template<typename String>
        auto to_string(String&& string) -> std::enable_if_t<__impl::is_string<String>, std::string> {
            return std::forward<String>(string);
        }
#elif __cplusplus == 201402L
        template<typename T>
        struct is_formatter_invokable {
        private:
            template<typename U, typename = std::enable_if_t<std::is_same<std::string,
                decltype(formatter<std::decay_t<U>>::to_string(std::declval<std::decay_t<U>>()))>::value>>
                static std::true_type test(void*);

            template<typename>
            static std::false_type test(...);
        public:
            static constexpr bool value = decltype(test<T>(nullptr))::value;
        };

        template<typename T>
        constexpr bool is_formatter_invokable_v = is_formatter_invokable<T>::value;

        template<typename T>
        std::enable_if_t<is_formatter_invokable_v<T>, std::string> to_string(const T& t) {
            return formatter<T>::to_string(t);
        }

        template<typename Numeric>
        auto to_string(Numeric numeric) -> decltype(std::to_string(numeric)) {
            return std::to_string(numeric);
        }

        template<typename String>
        std::enable_if_t<__impl::is_string<String>, std::string> to_string(String&& string) {
            return std::forward<String>(string);
        }
#endif


    }


    template<unsigned N>
    struct over_tuple {
        template<typename Tuple, typename Func>
        static void apply(Tuple&& tuple, Func&& func) {
            func(N, std::get<N>(std::forward<Tuple>(tuple)));
            over_tuple<N - 1>::apply(std::forward<Tuple>(tuple), std::forward<Func>(func));
        }
    };

    template<>
    struct over_tuple<0> {
        template<typename Tuple, typename Func>
        static void apply(Tuple&& tuple, Func&& func) {
            func(0, std::get<0>(tuple));
        }
    };

    template<typename... Args>
    std::string format_string(std::string&& message, Args&& ...args) {
        std::array<std::string, sizeof...(Args)> strings;
        {
            auto tuple = std::make_tuple(std::forward<Args>(args)...);
# if __cplusplus >= 201703L
            constexpr std::size_t size = std::tuple_size_v<decltype(tuple)> -1;
#elif __cplusplus == 201402L
            constexpr std::size_t size = std::tuple_size<decltype(tuple)>::value - 1;
#endif
            over_tuple<size>::apply(tuple, [&strings](std::size_t n, auto&& value) {
                strings[n] = helper::to_string(std::forward<decltype(value)>(value));
            });
        }

        std::string::size_type startIndex;
        std::string::size_type stopIndex;

        std::string::size_type i = 0;
        std::string result;
        result.reserve(message.size());
        bool needAdd = true;

        /// TODO(a.raag): if incorrect message format no throw
        for (; i < message.size(); ++i) {
            if (message[i] == '{') {
                startIndex = i;
                needAdd = false;
            }
            if (message[i] == '}') {
                stopIndex = i;
                int currentIndex = std::stoi(message.substr(startIndex + 1, stopIndex));
                const auto& substr = strings[currentIndex];

                result += substr;
                bool nextIsBraket = (message[stopIndex + 1] == '{');

                if (!nextIsBraket && i + 1 < message.size())
                    i = stopIndex + 1;
                else
                    i = stopIndex;

                if (!nextIsBraket && (i + 1) < message.size())
                    needAdd = true;
            }
            if (needAdd)
                result += message[i];
        }

        return result;
    }


}