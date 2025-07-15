#include <algorithm>
#include <array>
#include <fcntl.h>
#include <immintrin.h>
#include <numeric>
#include <stdint.h>
#include <string_view>
#include <sys/syscall.h>

#include "util.hpp"

namespace sr = std::ranges;

#ifndef MAX_ENTRIES
#define MAX_ENTRIES 256 * 1024
#endif

#ifndef MAX_SUB_ENTRIES
#define MAX_SUB_ENTRIES 256
#endif

using icon_type = std::array<char, 4>;
struct project {
    std::string_view f;
    icon_type icn;
    consteval project(const char *f_, const char (&icn_)[5])
        : f{f_},
          icn{icn_[0], icn_[1], icn_[2], icn_[3]}
    {
    }
};
constexpr inline std::array ps{
#ifdef ENABLE_PACKAGE_JSON
    project{"package.json", "📦"},
#endif
#ifdef ENABLE_PYPROJECT_TOML
    project{"pyproject.toml", "🐍"},
#endif
#ifdef ENABLE_REQUIREMENTS_TXT
    project{"requirements.txt", "🐍"},
#endif
#ifdef ENABLE_GO_MOD
    project{"go.mod", "🐹"},
#endif
#ifdef ENABLE_CARGO_TOML
    project{"Cargo.toml", "🦀"},
#endif
#ifdef ENABLE_POM_XML
    project{"pom.xml", "🧩"},
#endif
#ifdef ENABLE_BUILD_GRADLE
    project{"build.gradle", "🧩"},
#endif
#ifdef ENABLE_COMPOSER_JSON
    project{"composer.json", "🐘"},
#endif
#ifdef ENABLE_GEMFILE
    project{"Gemfile", "💎"},
#endif
#ifdef ENABLE_PUBSPEC_YAML
    project{"pubspec.yaml", "🎯"},
#endif
#ifdef ENABLE_CMAKELISTS_TXT
    project{"CMakeLists.txt", "🔧"},
#endif
#ifdef ENABLE_BUILD_ZIG
    project{"build.zig", "🧬"},
#endif
#ifdef ENABLE_MAKEFILE
    project{"Makefile", "🔨"},
#endif
    project{"", "📁"}};

#if LARGE_SUBS || !__AVX512VBMI__
constexpr inline auto faccess = true;
#else
constexpr inline auto faccess =
    ps.size() == 1 ||
    sr::fold_left(ps, 1uz, [](auto acc, const auto &p) { return acc + p.f.size() + 2; }) >= 64;
#endif

struct linux_dirent64 {
    unsigned long long d_ino;
    long long d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};

template <size_t I = 0>
inline icon_type icon([[maybe_unused]] int fd)
{
    if constexpr (I == ps.size() - 1)
        return ps.back().icn;
    else
        return (SC(faccessat, fd, ps[I].f.data(), F_OK) == 0) ? ps[I].icn : icon<I + 1>(fd);
}

[[noreturn]] icon_type icon(...) { std::unreachable(); }
template <size_t = 0>
inline auto icon([[maybe_unused]] linux_dirent64 &x)
{
    auto const [... is]       = util::mkidx_t<ps.size() - 1>{};

    static auto constexpr idx = [&] {
        std::array res{(ps[is].f.size() + 2)..., 1uz, 0uz};
        std::exclusive_scan(res.begin(), res.end(), res.begin(), 0uz);
        return res;
    }();

    static auto constexpr suf = [&] {
        std::array<char, 64> res{};
        (sr::copy(sr::views::iota(0uz, ps[is].f.size() + 1), &res[idx[is]]), ...);
        return std::bit_cast<__m512i>(res);
    }();
    auto const v             = _mm512_permutexvar_epi8(suf, _mm512_loadu_si512(x.d_name));

    static auto constexpr eq = [&] {
        std::array<char, 64> res{};
        (sr::copy(ps[is].f, &res[idx[is]]), ...);
        return std::bit_cast<__m512i>(res);
    }();
    auto m                    = _mm512_cmpeq_epi8_mask(v, eq);

    static auto constexpr add = ((1uz << idx[is]) | ...);
    m += add;

    static auto constexpr snt = ((1uz << (idx[is + 1] - 1)) | ...);
    m &= snt;

    static std::array<icon_type, ps.size() - 1> constexpr icns = {ps[is].icn...};
    static auto constexpr iidx                                 = [&] {
        std::array<uint8_t, idx[sizeof...(is)]> res{};
        ((res[idx[is + 1] - 1] = is), ...);
        return res;
    }();
    auto const ntz = __tzcnt_u64(m);
    struct R {
        decltype(ntz) ntz;
        inline explicit operator bool() const { return ntz != 64; }
        inline auto operator*() const { return icns[iidx[ntz]]; }
    };
    return R{ntz};
}

struct entry {
    alignas(64) char buf[64];
    long mtime;
    size_t nbuf;
};
entry g_es[MAX_ENTRIES];
char g_buf[(MAX_ENTRIES) * (sizeof(linux_dirent64) + 256)];

extern "C" {
[[gnu::used]] int main(int argc, char *argv[])
{
    auto const root = argc < 2 ? "." : argv[1];
    auto const rfd  = (int)SC(open, root, O_DIRECTORY | O_RDONLY | O_NOATIME);
    auto le         = &g_es[0];
    if (rfd < 0) goto error;

    {
        auto const nread = (long)SC(getdents64, rfd, g_buf, sizeof(g_buf));
        if (nread <= 0) goto error;
        for (auto pos = 0l; pos < nread;) {
            auto &entry = *(linux_dirent64 *)(void *)(g_buf + pos);
            DEFER [&] { pos += entry.d_reclen; };
            if (entry.d_name[0] == '.') continue;
            auto const sfd = (int)SC(openat, rfd, entry.d_name, O_DIRECTORY | O_RDONLY | O_NOATIME);
            if (sfd == -1) continue;
            DEFER [=] { SC(close, sfd); };

            // buf
            char sbuf[(MAX_SUB_ENTRIES) * (sizeof(entry) + 256)];
            auto const snread = faccess ? 0l : (long)SC(getdents64, sfd, sbuf, sizeof(sbuf));
            if (snread < 0) goto error;
            auto icn = faccess ? icon(sfd) : ps.back().icn;
            for (auto spos = 0l; spos < snread;) {
                auto &sentry = *(linux_dirent64 *)(void *)(sbuf + spos);
                DEFER [&] { spos += sentry.d_reclen; };
                if (auto icn_ = icon(sentry)) {
                    icn = *icn_;
                    break;
                }
            }
            auto dit      = sr::copy(icn, le->buf).out;
            *dit++        = ' ';
            auto const v  = _mm512_loadu_si512(entry.d_name);
            auto const vz = _mm512_set1_epi8('\0');
            auto const eq = _mm512_cmpeq_epu8_mask(v, vz);
            _mm512_storeu_si512(dit, v);
            auto const n = __tzcnt_u64(eq);
            dit += n;
            *dit++   = '\n';
            le->nbuf = (size_t)(dit - le->buf);

            // mtime
            struct stat st;
            SC(fstat, sfd, &st);
            le->mtime = st.st_mtim.tv_sec;
            ++le;
        }
    }

    if (g_es != le) {
        util::shell_sort(g_es, le, sr::greater{}, &entry::mtime);
        auto const out = std::as_writable_bytes(std::span{g_es}).data() + offsetof(entry, buf);
        auto pout      = out + g_es[0].nbuf;
        for (auto it = g_es + 1; it != le; ++it) {
            const auto &[buf, _, nbuf] = *it;
            _mm512_storeu_si512(pout, _mm512_load_si512(buf));
            pout += nbuf;
        }
        SC(write, 1, out, (size_t)(pout - out));
    }

    SC(exit, 0);
error:
    static constexpr util::string msg = "bad dir\n";
    SC(write, 2, msg.data(), msg.size());
    SC(exit, 1);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
[[noreturn, gnu::naked]] void _start()
{
    asm volatile( //
        "mov rdi, [rsp]\n"
        "lea rsi, [rsp+8]\n"
        "call main\n");
}
#pragma GCC diagnostic pop
}
