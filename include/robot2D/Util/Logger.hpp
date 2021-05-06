/*********************************************************************
(c) Alex Raag 2021
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
#include <iostream>

namespace logger {
    enum class logType {
        error,
        warn,
        info
    };

    extern bool debug;

    template<template<typename, typename ...> class Container,
            typename ValueType, typename ... Args>
    std::ostream &operator<<(std::ostream &os, const Container<ValueType, Args...> &c) {
        for (const auto &it: c)
            os << it;
        return os;
    }

    template<typename K, typename V>
    std::ostream &operator<<(std::ostream &os, const std::pair<K, V> &pair) {
        os << '[' << pair.first << ':' << pair.second << ']';
        return os;
    }

    void tprintf(logType type, const char *msg);

    template<typename T, typename ...Args>
    void tprintf(logType type, const char *format, const T &value, Args...args) {
        for (; *format != '\0'; format++) {
            if (*format == '%') {
                if (type == logType::info)
                    std::cout << value;
                if (type == logType::error)
                    std::cerr << value;
                tprintf(type, format + 1, args...);
                return;
            }
            if (type == logType::info)
                std::cout << *format;
            if (type == logType::error)
                std::cerr << *format;
        }
    }

    template<typename ...Args>
    void log(logType type, const char *msg, Args... args) {
        if (debug)
            tprintf(type, msg, args...);
    }

}

#define LOG_ERROR_E(msg, ...) logger::log(logger::logType::error, msg, #__VA_ARGS__);
#define LOG_ERROR(msg, ...) logger::log(logger::logType::error, msg, __VA_ARGS__);
#define LOG_WARN(msg, ...) logger::log(logger::logType::warn, msg, __VA_ARGS__);
#define LOG_INFO(msg, ...) logger::log(logger::logType::info, msg, __VA_ARGS__);
#define LOG_INFO_E(msg, ...) logger::log(logger::logType::info, msg, #__VA_ARGS__);
