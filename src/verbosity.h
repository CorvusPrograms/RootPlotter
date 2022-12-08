#pragma once
#include <fmt/format.h>
#include <iostream>
#include <cstdio>

extern int verbosity;

enum class VerbosityLevel : int {
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
};

template <typename... Args>
void vPrint(VerbosityLevel level, std::string_view format, Args&&... args) {
    if (static_cast<int>(level) <= verbosity) {
        fmt::print(stderr, format, args...);
    }
}

template <typename... Args>
void vRuntimeError(std::string_view format, Args&&... args) {
    throw std::runtime_error(fmt::format(format, args...));
}
