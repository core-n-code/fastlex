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

using char_bitmap = std::array<std::uint64_t, 4>;

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
{ return ((table[x >> 6] >> (x & 63u)) & 1u) != 0u; }

// clang-format off
static constexpr char_bitmap alnum_chars = make_bitmap("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
static constexpr char_bitmap alpha_chars = make_bitmap("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
static constexpr char_bitmap blank_chars = make_bitmap("\t ");
// we have to use an explicit string_view with length here because of \000
static constexpr char_bitmap cntrl_chars = make_bitmap(std::string_view{"\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\177",33});
static constexpr char_bitmap digit_chars = make_bitmap("0123456789");
static constexpr char_bitmap graph_chars = make_bitmap(R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)");
static constexpr char_bitmap lower_chars = make_bitmap("abcdefghijklmnopqrstuvwxyz");
static constexpr char_bitmap print_chars = make_bitmap(R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)");
static constexpr char_bitmap punct_chars = make_bitmap(R"(!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)");
static constexpr char_bitmap space_chars = make_bitmap("\t\n\v\f\r ");
static constexpr char_bitmap upper_chars = make_bitmap("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
static constexpr char_bitmap xdigit_chars = make_bitmap("0123456789ABCDEFabcdef");
// clang-format on

namespace detail {
[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto range_mask(unsigned char x,
                                                              unsigned char first,
                                                              unsigned char last) noexcept
    -> unsigned char
{
    auto const in_range = static_cast<unsigned>(
        (static_cast<unsigned>(x) - static_cast<unsigned>(first)) <=
        (static_cast<unsigned>(last) - static_cast<unsigned>(first)));

    return static_cast<unsigned char>(0u - in_range);
}
} // namespace detail

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isalnum(unsigned char x) noexcept -> bool
{ return bitmap_contains(alnum_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isalpha(unsigned char x) noexcept -> bool
{ return bitmap_contains(alpha_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isblank(unsigned char x) noexcept -> bool
{ return bitmap_contains(blank_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto iscntrl(unsigned char x) noexcept -> bool
{ return bitmap_contains(cntrl_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isdigit(unsigned char x) noexcept -> bool
{ return bitmap_contains(digit_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isgraph(unsigned char x) noexcept -> bool
{ return bitmap_contains(graph_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto islower(unsigned char x) noexcept -> bool
{ return bitmap_contains(lower_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isprint(unsigned char x) noexcept -> bool
{ return bitmap_contains(print_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto ispunct(unsigned char x) noexcept -> bool
{ return bitmap_contains(punct_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isspace(unsigned char x) noexcept -> bool
{ return bitmap_contains(space_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isupper(unsigned char x) noexcept -> bool
{ return bitmap_contains(upper_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isxdigit(unsigned char x) noexcept -> bool
{ return bitmap_contains(xdigit_chars, x); }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto isascii(unsigned char x) noexcept -> bool
{ return x <= 0x7fu; }

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto tolower(unsigned char x) noexcept
    -> unsigned char
{
    auto const upper = detail::range_mask(x, 'A', 'Z');
    return static_cast<unsigned char>(x | (upper & 0x20u));
}

[[nodiscard]] FASTLEX_ALWAYS_INLINE constexpr auto toupper(unsigned char x) noexcept
    -> unsigned char
{
    auto const lower = detail::range_mask(x, 'a', 'z');
    return static_cast<unsigned char>(x & ~(lower & 0x20u));
}

} // namespace fastlex::ascii

#undef FASTLEX_ALWAYS_INLINE
