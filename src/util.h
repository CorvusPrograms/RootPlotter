#pragma once

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

#define BUILD(C, var)                                                         \
#var,                                                                     \
        sol::overload(                                                        \
            [](C *c, const underlying_optional <decltype(C::var)>::type &v) { \
                c->var = v;                                                   \
                return c;                                                     \
            },                                                                \
            [](C *c) { return c->var; })
