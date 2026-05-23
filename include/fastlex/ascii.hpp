#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#if defined(__GNUC__) || defined(__clang__)
#define FASTLEX_ALWAYS_INLINE [[gnu::always_inline]] inline
#else
#define FASTLEX_ALWAYS_INLINE inline
#endif

namespace fastlex::ascii {

enum class matcher_backend : std::uint8_t { BITMAP, LOOKUP };

namespace detail {

using char_bitmap = std::array<std::uint64_t, 4>;
using char_map = std::array<unsigned char, 256>;

[[nodiscard]] consteval auto make_bitmap(std::string_view chars) noexcept -> char_bitmap
{
    char_bitmap table{};

    for(unsigned char c : chars) {
        table[c >> 6] |= std::uint64_t{1} << (c & 63u);
    }

    return table;
}

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto bitmap_contains(char_bitmap const& table,
                                                                   unsigned char x) noexcept -> bool
{ return (table[x >> 6] & (std::uint64_t{1} << (x & 63u))) != 0u; }

[[nodiscard]] consteval auto make_lookup(char_bitmap bitmap) noexcept -> char_map
{
    char_map table{};

    for(auto i = 0u; i < table.size(); ++i) {
        auto const x = static_cast<unsigned char>(i);
        table[i] = bitmap_contains(bitmap, x) ? 1u : 0u;
    }

    return table;
}

template<matcher_backend Backend>
struct custom_matcher;

template<>
struct alignas(64) custom_matcher<matcher_backend::BITMAP>
{
    constexpr explicit custom_matcher(char_bitmap bitmap) noexcept : bitmap_(bitmap) {}

    [[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto operator()(unsigned char x) const noexcept
        -> bool
    { return bitmap_contains(bitmap_, x); }

private:
    char_bitmap bitmap_;
};

template<>
struct alignas(64) custom_matcher<matcher_backend::LOOKUP>
{
    constexpr explicit custom_matcher(char_map lookup) noexcept : lookup_(lookup) {}

    [[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto operator()(unsigned char x) const noexcept
        -> bool
    { return lookup_[x] != 0u; }

private:
    char_map lookup_;
};

template<matcher_backend Backend>
[[nodiscard]] consteval auto make_matcher(std::string_view chars) noexcept
{
    auto const bitmap = make_bitmap(chars);

    if constexpr(Backend == matcher_backend::BITMAP) {
        return custom_matcher<Backend>{bitmap};
    } else {
        return custom_matcher<Backend>{make_lookup(bitmap)};
    }
}

[[nodiscard]] consteval auto make_lower_map() noexcept -> char_map
{
    char_map table{};

    for(auto i = 0u; i < table.size(); ++i) {
        auto const x = static_cast<unsigned char>(i);
        table[i] = (x >= 'A' && x <= 'Z') ? static_cast<unsigned char>(x + 0x20u) : x;
    }

    return table;
}

[[nodiscard]] consteval auto make_upper_map() noexcept -> char_map
{
    char_map table{};

    for(auto i = 0u; i < table.size(); ++i) {
        auto const x = static_cast<unsigned char>(i);
        table[i] = (x >= 'a' && x <= 'z') ? static_cast<unsigned char>(x - 0x20u) : x;
    }

    return table;
}

alignas(64) static constexpr char_map lower_map = make_lower_map();
alignas(64) static constexpr char_map upper_map = make_upper_map();

} // namespace detail

template<matcher_backend Backend = matcher_backend::BITMAP>
[[nodiscard]] consteval auto custom(std::string_view chars) noexcept
{ return detail::make_matcher<Backend>(chars); }


template<matcher_backend BACKEND = matcher_backend::LOOKUP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isalnum(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::LOOKUP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isalpha(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::LOOKUP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isblank(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "\t "sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::LOOKUP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto iscntrl(unsigned char x) noexcept -> bool
{
    // clang-format off
    static constexpr auto alphabet = std::string_view{
        "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017"
        "\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\177",
        33};
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::BITMAP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isdigit(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "0123456789"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::BITMAP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isgraph(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::BITMAP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto islower(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "abcdefghijklmnopqrstuvwxyz"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::BITMAP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isprint(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::LOOKUP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto ispunct(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = R"(!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::LOOKUP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isspace(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "\t\n\v\f\r "sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::BITMAP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isupper(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

template<matcher_backend BACKEND = matcher_backend::LOOKUP>
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isxdigit(unsigned char x) noexcept -> bool
{
    // clang-format off
    using std::string_view_literals::operator""sv;
    static constexpr auto alphabet = "0123456789ABCDEFabcdef"sv;
    static constexpr auto matcher = detail::make_matcher<BACKEND>(alphabet);
    // clang-format on

    return matcher(x);
}

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isascii(unsigned char x) noexcept -> bool
{ return x <= 0x7fu; }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto tolower(unsigned char x) noexcept
    -> unsigned char
{ return detail::lower_map[x]; }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto toupper(unsigned char x) noexcept
    -> unsigned char
{ return detail::upper_map[x]; }

} // namespace fastlex::ascii

#undef FASTLEX_ALWAYS_INLINE
