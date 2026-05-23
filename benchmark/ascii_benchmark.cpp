#include <fastlex/ascii.hpp>

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>

#include <benchmark/benchmark.h>

namespace {

constexpr std::size_t data_size = 1 << 20;

auto make_data() -> std::array<unsigned char, data_size>
{
    std::array<unsigned char, data_size> data{};
    std::uint32_t state = 0x12345678u;

    for(auto& byte : data) {
        state = (state * 1664525u) + 1013904223u;
        byte = static_cast<unsigned char>(state >> 24);
    }

    return data;
}

auto const data = make_data();

void set_byte_counters(benchmark::State& state)
{
    auto const bytes_per_iteration = static_cast<std::int64_t>(data.size());

    state.SetBytesProcessed(state.iterations() * bytes_per_iteration);
    state.counters["time_per_byte"] = benchmark::Counter(
        data.size(),
        benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kInvert);
}

template<typename Predicate>
void classify_all(benchmark::State& state, Predicate predicate)
{
    for(auto _ : state) {
        std::size_t count = 0;
        for(auto byte : data) {
            count += predicate(byte) ? 1u : 0u;
        }
        benchmark::DoNotOptimize(count);
    }

    set_byte_counters(state);
}

// clang-format off
constexpr auto branch_isalnum(unsigned char c) -> bool { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
constexpr auto branch_isalpha(unsigned char c) -> bool { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
constexpr auto branch_isdigit(unsigned char c) -> bool { return c >= '0' && c <= '9'; }
constexpr auto branch_isblank(unsigned char c) -> bool { return c == '\t' || c == ' '; }
constexpr auto branch_iscntrl(unsigned char c) -> bool { return c <= 0x1fu || c == 0x7fu; }
constexpr auto branch_isgraph(unsigned char c) -> bool { return c >= 0x21u && c <= 0x7eu; }
constexpr auto branch_islower(unsigned char c) -> bool { return c >= 'a' && c <= 'z'; }
constexpr auto branch_isprint(unsigned char c) -> bool { return c >= 0x20u && c <= 0x7eu; }
constexpr auto branch_ispunct(unsigned char c) -> bool { return branch_isgraph(c) && !branch_isalnum(c); }
constexpr auto branch_isspace(unsigned char c) -> bool { return c == ' ' || (c >= '\t' && c <= '\r'); }
constexpr auto branch_isupper(unsigned char c) -> bool { return c >= 'A' && c <= 'Z'; }
constexpr auto branch_isxdigit(unsigned char c) -> bool { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
constexpr auto branch_tolower(unsigned char c) -> unsigned char { return branch_isupper(c) ? static_cast<unsigned char>(c + ('a' - 'A')) : c; }
constexpr auto branch_toupper(unsigned char c) -> unsigned char { return branch_islower(c) ? static_cast<unsigned char>(c - ('a' - 'A')) : c; }
// clang-format off

#define FASTLEX_DEFINE_CLASS_BENCHMARKS(name, fastlex_expr, branch_expr, std_expr) \
    void BM_fastlex_##name(benchmark::State& state) \
    { \
        classify_all(state, [](unsigned char c) { return (fastlex_expr); }); \
    } \
\
    void BM_branch_##name(benchmark::State& state) \
    { \
        classify_all(state, [](unsigned char c) { return (branch_expr); }); \
    } \
\
    void BM_std_##name(benchmark::State& state) \
    { \
        classify_all(state, [](unsigned char c) { return (std_expr); }); \
    }

FASTLEX_DEFINE_CLASS_BENCHMARKS(isalnum,
                                fastlex::ascii::isalnum(c),
                                branch_isalnum(c),
                                std::isalnum(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isalpha,
                                fastlex::ascii::isalpha(c),
                                branch_isalpha(c),
                                std::isalpha(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isblank,
                                fastlex::ascii::isblank(c),
                                branch_isblank(c),
                                std::isblank(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(iscntrl,
                                fastlex::ascii::iscntrl(c),
                                branch_iscntrl(c),
                                std::iscntrl(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isdigit,
                                fastlex::ascii::isdigit(c),
                                branch_isdigit(c),
                                std::isdigit(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isgraph,
                                fastlex::ascii::isgraph(c),
                                branch_isgraph(c),
                                std::isgraph(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(islower,
                                fastlex::ascii::islower(c),
                                branch_islower(c),
                                std::islower(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isprint,
                                fastlex::ascii::isprint(c),
                                branch_isprint(c),
                                std::isprint(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(ispunct,
                                fastlex::ascii::ispunct(c),
                                branch_ispunct(c),
                                std::ispunct(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isspace,
                                fastlex::ascii::isspace(c),
                                branch_isspace(c),
                                std::isspace(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isupper,
                                fastlex::ascii::isupper(c),
                                branch_isupper(c),
                                std::isupper(c) != 0)
FASTLEX_DEFINE_CLASS_BENCHMARKS(isxdigit,
                                fastlex::ascii::isxdigit(c),
                                branch_isxdigit(c),
                                std::isxdigit(c) != 0)

#undef FASTLEX_DEFINE_CLASS_BENCHMARKS

template<typename Converter>
void convert_all(benchmark::State& state, Converter converter)
{
    for(auto _ : state) {
        std::uint64_t sum = 0;
        for(auto byte : data) {
            sum += converter(byte);
        }
        benchmark::DoNotOptimize(sum);
    }
    set_byte_counters(state);
}

void BM_fastlex_tolower(benchmark::State& state)
{
    convert_all(state, [](unsigned char c) { return fastlex::ascii::tolower(c); });
}

void BM_branch_tolower(benchmark::State& state)
{
    convert_all(state, [](unsigned char c) { return branch_tolower(c); });
}

void BM_std_tolower(benchmark::State& state)
{
    convert_all(state, [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
}

void BM_fastlex_toupper(benchmark::State& state)
{
    convert_all(state, [](unsigned char c) { return fastlex::ascii::toupper(c); });
}

void BM_branch_toupper(benchmark::State& state)
{
    convert_all(state, [](unsigned char c) { return branch_toupper(c); });
}

void BM_std_toupper(benchmark::State& state)
{
    convert_all(state, [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
}

} // namespace

BENCHMARK(BM_fastlex_isalnum);
BENCHMARK(BM_branch_isalnum);
BENCHMARK(BM_std_isalnum);
BENCHMARK(BM_fastlex_isalpha);
BENCHMARK(BM_branch_isalpha);
BENCHMARK(BM_std_isalpha);
BENCHMARK(BM_fastlex_isblank);
BENCHMARK(BM_branch_isblank);
BENCHMARK(BM_std_isblank);
BENCHMARK(BM_fastlex_iscntrl);
BENCHMARK(BM_branch_iscntrl);
BENCHMARK(BM_std_iscntrl);
BENCHMARK(BM_fastlex_isdigit);
BENCHMARK(BM_branch_isdigit);
BENCHMARK(BM_std_isdigit);
BENCHMARK(BM_fastlex_isgraph);
BENCHMARK(BM_branch_isgraph);
BENCHMARK(BM_std_isgraph);
BENCHMARK(BM_fastlex_islower);
BENCHMARK(BM_branch_islower);
BENCHMARK(BM_std_islower);
BENCHMARK(BM_fastlex_isprint);
BENCHMARK(BM_branch_isprint);
BENCHMARK(BM_std_isprint);
BENCHMARK(BM_fastlex_ispunct);
BENCHMARK(BM_branch_ispunct);
BENCHMARK(BM_std_ispunct);
BENCHMARK(BM_fastlex_isspace);
BENCHMARK(BM_branch_isspace);
BENCHMARK(BM_std_isspace);
BENCHMARK(BM_fastlex_isupper);
BENCHMARK(BM_branch_isupper);
BENCHMARK(BM_std_isupper);
BENCHMARK(BM_fastlex_isxdigit);
BENCHMARK(BM_branch_isxdigit);
BENCHMARK(BM_std_isxdigit);
BENCHMARK(BM_fastlex_tolower);
BENCHMARK(BM_branch_tolower);
BENCHMARK(BM_std_tolower);
BENCHMARK(BM_fastlex_toupper);
BENCHMARK(BM_branch_toupper);
BENCHMARK(BM_std_toupper);

BENCHMARK_MAIN();
