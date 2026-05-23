# fastlex
[![CI](https://github.com/core-n-code/fastlex/actions/workflows/ci.yml/badge.svg)](https://github.com/core-n-code/fastlex/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)



`fastlex` is a tiny C++23 header-only library for fast ASCII character
classification and ASCII case conversion.

It is built for lexer, parser, tokenizer, and protocol hot paths where the input
is bytes and the grammar is ASCII. In those places, the usual `cctype` functions
often do more work than needed: they accept `int`, have locale-shaped semantics,
and are not designed around compile-time custom character classes.

`fastlex` keeps the domain explicit:

```text
input byte:  unsigned char
domain:      0..255
semantics:   ASCII only
tables:      generated at compile time
dispatch:    branchless
```

## Quick Start

Include the library:

```cpp
#include <fastlex/ascii.hpp>
```

Use the `cctype`-style API for explicit ASCII classification:

```cpp
fastlex::ascii::isalpha('A');
fastlex::ascii::isdigit('7');
fastlex::ascii::isspace('\n');
fastlex::ascii::isxdigit('f');
fastlex::ascii::tolower('Q'); // returns unsigned char{'q'}
```

The full ASCII `cctype`-style set is available:

```text
isalnum   isalpha   isblank   iscntrl
isascii   isdigit   isgraph   islower
isprint   ispunct   isspace   isupper
isxdigit  tolower   toupper
```

The functions live in `fastlex::ascii` and intentionally take `unsigned char`.
If your input is a `char`, cast once at the boundary of your scanner:

```cpp
auto const byte = static_cast<unsigned char>(input[pos]);

if(fastlex::ascii::isalpha(byte)) {
    // ...
}
```

## Custom Matchers

Build your own compile-time character classes with `custom`:

```cpp
using namespace std::literals;

constexpr auto separators = fastlex::ascii::custom(",:;|"sv);

static_assert(separators(','));
static_assert(!separators('x'));
```

The argument is a `std::string_view` containing the accepted byte values.
`custom` is `consteval`, so the matcher table is built during compilation and
the returned object is a tiny `constexpr` callable.

Longer classes are just longer strings:

```cpp
constexpr auto ident_start =
    fastlex::ascii::custom("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "_"sv);
```

Repeated characters are harmless. A character class is a set, so putting the
same byte into the string twice sets the same table entry twice and does not
change the result:

```cpp
constexpr auto digits = fastlex::ascii::custom("00112233445566778899"sv);

static_assert(digits('7'));
static_assert(!digits('x'));
```

## Matcher Backends

Custom matchers and most built-in predicates can use one of two backends:

```cpp
enum class matcher_backend : std::uint8_t {
    BITMAP,
    LOOKUP,
};
```

`BITMAP` is the default for `custom`:

```cpp
constexpr auto compact =
    fastlex::ascii::custom<fastlex::ascii::matcher_backend::BITMAP>("0123456789"sv);
```

It stores the whole 256-byte domain in four `std::uint64_t` words, so the table
is only 32 bytes:

```cpp
return (table[x >> 6] & (std::uint64_t{1} << (x & 63u))) != 0u;
```

The bitmap backend is compact and usually a great fit for small, simple classes.
The cost per query is a word load, a shift, an `and`, and a comparison.

`LOOKUP` stores one byte per possible input value:

```cpp
constexpr auto direct =
    fastlex::ascii::custom<fastlex::ascii::matcher_backend::LOOKUP>("0123456789"sv);
```

The lookup table is 256 bytes and contains `0` or `1` for each byte:

```cpp
return lookup[x] != 0u;
```

The lookup backend uses more memory, but the query is just an indexed byte load
and a comparison. It can win for larger, compound, or irregular classes where
avoiding bitmap arithmetic matters more than the extra table size.

As a rule of thumb:

```text
BITMAP  smaller table, excellent for compact/simple classes
LOOKUP  larger table, often faster for compound/irregular classes
```

The best choice is workload and compiler dependent, so the built-in predicates
choose defaults from benchmarks, and callers can override those defaults.

## Built-In Predicates

The built-ins are implemented with the same matcher machinery that powers
`custom`. Each predicate owns a C++23 local `static constexpr` matcher, so the
class definition stays next to the function while the table is still generated
at compile time.

For example, `isdigit` is conceptually:

```cpp
template<fastlex::ascii::matcher_backend BACKEND = fastlex::ascii::matcher_backend::BITMAP>
constexpr auto isdigit(unsigned char x) noexcept -> bool
{
    using namespace std::literals;
    static constexpr auto matcher = detail::make_matcher<BACKEND>("0123456789"sv);
    return matcher(x);
}
```

Predicates can be instantiated with an explicit backend:

```cpp
fastlex::ascii::isdigit<fastlex::ascii::matcher_backend::BITMAP>('7');
fastlex::ascii::isalnum<fastlex::ascii::matcher_backend::LOOKUP>('A');
```

Current defaults:

```text
LOOKUP  isalnum isalpha isblank iscntrl ispunct isspace isxdigit
BITMAP  isdigit isgraph islower isprint isupper
```

`isascii` does not need a table:

```cpp
return x <= 0x7f;
```

## Case Conversion

`tolower` and `toupper` use direct 256-entry byte maps:

```cpp
return lower_map[x];
return upper_map[x];
```

That makes case conversion branchless for every byte. Bytes outside `A-Z` for
`tolower` and outside `a-z` for `toupper` map to themselves.

## How The Tables Are Built

Every character class starts as a `std::string_view` of accepted bytes.
`make_bitmap` turns that string into a 32-byte bitmap:

```cpp
char_bitmap table{};

for(unsigned char c : chars) {
    table[c >> 6] |= std::uint64_t{1} << (c & 63u);
}
```

If the requested backend is `BITMAP`, that bitmap becomes the matcher storage.
If the requested backend is `LOOKUP`, `make_lookup` expands the bitmap into a
256-byte `0/1` table.

The important detail is that all of this happens at compile time:

```text
custom("..."sv)
    -> consteval make_bitmap(...)
    -> optional consteval make_lookup(...)
    -> constexpr matcher object
    -> branchless operator()(unsigned char)
```

There are three table shapes:

```text
bitmap matcher  4 x uint64_t          32 bytes
lookup matcher  256 x unsigned char   256 bytes
case map        256 x unsigned char   256 bytes
```

## ASCII Semantics

`fastlex` is deliberately not locale-aware. For example, `isalpha` means only:

```text
A-Z
a-z
```

Bytes `0x80..0xff` are not treated as alphabetic, printable, punctuation, or
space. Case conversion only changes bytes in `A-Z` or `a-z`.

`isxdigit` follows `cctype`:

```text
0-9
A-F
a-f
```

## CMake

Use the exported interface target:

```cmake
find_package(fastlex CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE fastlex::fastlex)
```

From this repository:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DFASTLEX_BUILD_TESTS=ON -DFASTLEX_BUILD_BENCHMARKS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

CMake options:

```text
FASTLEX_BUILD_TESTS       Build gtest coverage
FASTLEX_BUILD_BENCHMARKS  Build Google Benchmark executables
FASTLEX_BUILD_EXAMPLES    Build the small example program
```

## Benchmarks

Benchmarks live in `benchmark/ascii_benchmark.cpp` and compare:

```text
fastlex branchless predicates
branch-based ASCII predicates
std::cctype predicates
```

Run them after building:

```sh
./build/benchmark/fastlex_ascii_benchmark
```

These numbers were measured with:

```text
CPU:       AMD Ryzen 9 7950X
OS:        NixOS, Linux 7.0.6
Compiler:  clang 22.1.5
Command:   ./build/benchmark/fastlex_ascii_benchmark --benchmark_min_time=0.05s --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
```

```text
Predicate   fastlex    branch     std::cctype
isalnum     5.01 GB/s  1.94 GB/s  547 MB/s
isalpha     4.98 GB/s  2.99 GB/s  547 MB/s
isblank     3.97 GB/s  3.97 GB/s  498 MB/s
iscntrl     5.02 GB/s  3.31 GB/s  547 MB/s
isdigit     3.90 GB/s  3.26 GB/s  3.26 GB/s
isgraph     3.90 GB/s  3.26 GB/s  547 MB/s
islower     3.92 GB/s  3.27 GB/s  607 MB/s
isprint     3.82 GB/s  3.25 GB/s  546 MB/s
ispunct     4.99 GB/s  1.83 GB/s  545 MB/s
isspace     4.99 GB/s  3.03 GB/s  545 MB/s
isupper     3.90 GB/s  3.26 GB/s  547 MB/s
isxdigit    5.00 GB/s  1.94 GB/s  547 MB/s
tolower     7.01 GB/s  3.27 GB/s  497 MB/s
toupper     7.01 GB/s  3.26 GB/s  495 MB/s
```

## Nix

Enter the development shell:

```sh
nix develop
```

Build and run checks through the flake:

```sh
nix build
nix flake check
```
