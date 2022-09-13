#include <fmt/format.h>

extern int verbosity;

enum VerbosityLevel {
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
};

template <typename... Args>
void vPrint(std::string_view format, VerbosityLevel level, Args&&... args) {
    if (level <= verbosity) {
        fmt::print(format, args...);
    }
}

template <typename... Args>
void vPrintLow(std::string_view format,  Args&&... args) {
    vPrint(format, VerbosityLevel::Low, args...);
}
template <typename... Args>
void vPrintMedium(std::string_view format,  Args&&... args) {
    vPrint(format, VerbosityLevel::Medium, args...);
}

template <typename... Args>
void vPrintHigh(std::string_view format,  Args&&... args) {
    vPrint(format, VerbosityLevel::High, args...);
}


template <typename... Args>
void vRuntimeError(std::string_view format,  Args&&... args) {
    throw std::runtime_error(fmt::format(format, args...));
}
