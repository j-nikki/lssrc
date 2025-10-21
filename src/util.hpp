#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>
#include <utility>

namespace util
{
#define CAT(A, B)               _CAT_EXP(A, B)
#define _CAT_EXP(A, B)          A##B

#define _SC_ARGINIT(Arg0, ...)  __VA_OPT__(_SC_ARGINIT1(__VA_ARGS__))
#define _SC_ARGINIT1(Arg1, ...) __VA_OPT__(_SC_ARGINIT2(__VA_ARGS__))
#define _SC_ARGINIT2(Arg2, ...) __VA_OPT__(_SC_ARGINIT3(__VA_ARGS__))
#define _SC_ARGINIT3(Arg3, ...)                                                                    \
    register auto r10 asm("r10") = (Arg3);                                                         \
    __VA_OPT__(_SC_ARGINIT4(__VA_ARGS__))
#define _SC_ARGINIT4(Arg4, ...)                                                                    \
    register auto r8 asm("r8") = (Arg4);                                                           \
    __VA_OPT__(_SC_ARGINIT5(__VA_ARGS__))
#define _SC_ARGINIT5(Arg5, ...) register auto r9 asm("r9") = (Arg5);

#define _SC_ARGIN(Arg0, ...)    "D"(Arg0)__VA_OPT__(, _SC_ARGIN1(__VA_ARGS__))
#define _SC_ARGIN1(Arg1, ...)   "S"(Arg1)__VA_OPT__(, _SC_ARGIN2(__VA_ARGS__))
#define _SC_ARGIN2(Arg2, ...)   "d"(Arg2)__VA_OPT__(, _SC_ARGIN3(__VA_ARGS__))
#define _SC_ARGIN3(Arg3, ...)   "r"(r10)__VA_OPT__(, _SC_ARGIN4(__VA_ARGS__))
#define _SC_ARGIN4(Arg4, ...)   "r"(r8)__VA_OPT__(, _SC_ARGIN5(__VA_ARGS__))
#define _SC_ARGIN5(Arg5, ...)   "r"(r9)

#define _SC_ARGOUT(Arg0, ...)   "=D"(CAT(ignored, __LINE__))__VA_OPT__(, _SC_ARGOUT1(__VA_ARGS__))
#define _SC_ARGOUT1(Arg1, ...)  "=S"(CAT(ignored, __LINE__))__VA_OPT__(, _SC_ARGOUT2(__VA_ARGS__))
#define _SC_ARGOUT2(Arg2, ...)  "=d"(CAT(ignored, __LINE__))__VA_OPT__(, _SC_ARGOUT3(__VA_ARGS__))
#define _SC_ARGOUT3(Arg3, ...)  "=r"(CAT(ignored, __LINE__))__VA_OPT__(, _SC_ARGOUT4(__VA_ARGS__))
#define _SC_ARGOUT4(Arg4, ...)  "=r"(CAT(ignored, __LINE__))__VA_OPT__(, _SC_ARGOUT5(__VA_ARGS__))
#define _SC_ARGOUT5(Arg5, ...)  "=r"(CAT(ignored, __LINE__))

#define SC(Name, ...)                                                                              \
    ({                                                                                             \
        auto CAT(sc_ret, __LINE__) = (intptr_t)SYS_##Name;                                         \
        [[maybe_unused]] void *CAT(ignored, __LINE__);                                             \
        _SC_ARGINIT(__VA_ARGS__)                                                                   \
        asm volatile("syscall"                                                                     \
                     : "+a"(CAT(sc_ret, __LINE__))__VA_OPT__(, _SC_ARGOUT(__VA_ARGS__))            \
                     : __VA_OPT__(_SC_ARGIN(__VA_ARGS__))                                          \
                     : "memory", "rcx", "r11");                                                    \
        if (SYS_##Name == SYS_exit || SYS_##Name == SYS_exit_group) std::unreachable();            \
        CAT(sc_ret, __LINE__);                                                                     \
    })

template <size_t N, class Map = std::identity, class Ics = std::make_index_sequence<N>>
struct mkidx;
template <size_t N, class Map, size_t... Is>
struct mkidx<N, Map, std::index_sequence<Is...>>
    : std::type_identity<std::tuple<std::integral_constant<size_t, Map{}(Is)>...>> {};
template <size_t N, class Map = std::identity>
using mkidx_t = typename mkidx<N, Map>::type;

template <std::random_access_iterator I, std::sentinel_for<I> S, class P = std::identity,
          std::indirect_strict_weak_order<std::projected<I, P>> C = std::ranges::less>
void shell_sort(I f, S l, C c = {}, P p = {})
{
    auto const n = (size_t)std::ranges::distance(f, l);
    for (auto gap = n / 2; gap > 0; gap /= 2) {
        for (auto i = gap; i < n; i++) {
            auto temp = f[i];
            size_t j;
            for (j = i;
                 j >= gap && std::invoke(c, std::invoke(p, temp), std::invoke(p, f[j - gap]));
                 j -= gap) {
                f[j] = f[j - gap];
            }
            f[j] = temp;
        }
    }
}

template <size_t N>
struct string : std::array<char, N> {
    consteval string(char const (&cs)[N + 1]) { std::ranges::copy_n(cs, N, this->data()); }
};
template <size_t N>
string(char const (&cs)[N]) -> string<N - 1>;

namespace detail
{
template <class F>
struct defer {
    F f_;
    constexpr ~defer() { f_(); }
};
struct defer_proxy_t {
    template <class F>
    constexpr defer<F> operator=(F &&f) const
    {
        return {static_cast<F &&>(f)};
    }
};
constexpr inline defer_proxy_t defer_proxy;
} // namespace detail
#define DEFER auto CAT(defer, __LINE__) = ::util::detail::defer_proxy =
} // namespace util
