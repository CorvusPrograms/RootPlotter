#pragma once
#include <optional>

template <typename T, typename X = void>
struct underlying_optional {
    using type = T;
};
template <typename T>
struct underlying_optional<
    T, typename std::enable_if<
           std::is_same_v<std::optional<typename T::value_type>, T>>::type> {
    using type = typename T::value_type;
};

#define BUILD(C, var) \
    #var, sol::overload(                                                  \
              [](sol::object c,                                           \
                 const underlying_optional <decltype(C::var)>::type &v) { \
                  c.as<C>().var = v;                                      \
                  return c;                                               \
              },                                                          \
              [](sol::object c)->decltype(c.as<C>().var)&{ return c.as<C>().var; })

template <typename T, typename F>
void maybe_fun(const std::optional<T> &opt, F &&f) {
    if (opt) {
        f(opt.value());
    }
}
template <typename T, typename F, typename F2>
void maybe_fun(const std::optional<T> &opt, F &&f, F2 &&f2) {
    if (opt) {
        f(opt.value());
    } else {
        f2();
    }
}

template <typename T, typename U>
void maybe_assign(const std::optional<T> &opt, U &val) {
    if (opt) {
        val = opt.value();
    }
}
