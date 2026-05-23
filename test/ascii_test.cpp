#include <fastlex/ascii.hpp>

#include <string_view>

#include <gtest/gtest.h>

namespace {

using namespace std::literals;

// clang-format off
constexpr auto between(int c, int lo, int hi) -> bool { return c >= lo && c <= hi; }
constexpr auto expect_digit(int c) -> bool { return between(c, '0', '9'); }
constexpr auto expect_upper(int c) -> bool { return between(c, 'A', 'Z'); }
constexpr auto expect_lower(int c) -> bool { return between(c, 'a', 'z'); }
constexpr auto expect_alpha(int c) -> bool { return expect_upper(c) || expect_lower(c); }
constexpr auto expect_alnum(int c) -> bool { return expect_alpha(c) || expect_digit(c); }
constexpr auto expect_blank(int c) -> bool { return c == '\t' || c == ' '; }
constexpr auto expect_cntrl(int c) -> bool { return between(c, 0x00, 0x1f) || c == 0x7f; }
constexpr auto expect_graph(int c) -> bool { return between(c, 0x21, 0x7e); }
constexpr auto expect_print(int c) -> bool { return between(c, 0x20, 0x7e); }
constexpr auto expect_space(int c) -> bool { return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' '; }
constexpr auto expect_xdigit(int c) -> bool { return expect_digit(c) || between(c, 'A', 'F') || between(c, 'a', 'f'); }
constexpr auto expect_punct(int c) -> bool { return expect_graph(c) && !expect_alnum(c); }
// clang-format on

constexpr auto expect_lowered(int c) -> unsigned char
{
    return expect_upper(c) ? static_cast<unsigned char>(c + ('a' - 'A'))
                           : static_cast<unsigned char>(c);
}

constexpr auto expect_uppered(int c) -> unsigned char
{
    return expect_lower(c) ? static_cast<unsigned char>(c - ('a' - 'A'))
                           : static_cast<unsigned char>(c);
}

template<typename Predicate>
constexpr auto all_bytes(Predicate predicate) -> bool
{
    for(int c = 0; c <= 255; ++c) {
        if(!predicate(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

// clang-format off
static_assert(fastlex::ascii::custom("az"sv)('a'));
static_assert(fastlex::ascii::custom("az"sv)('z'));
static_assert(!fastlex::ascii::custom("az"sv)('b'));
static_assert(fastlex::ascii::custom<fastlex::ascii::matcher_backend::BITMAP>("az"sv)('a'));
static_assert(!fastlex::ascii::custom<fastlex::ascii::matcher_backend::BITMAP>("az"sv)('b'));
static_assert(fastlex::ascii::custom<fastlex::ascii::matcher_backend::LOOKUP>("az"sv)('a'));
static_assert(!fastlex::ascii::custom<fastlex::ascii::matcher_backend::LOOKUP>("az"sv)('b'));
static_assert(fastlex::ascii::isalnum('Q'));
static_assert(fastlex::ascii::isalnum<fastlex::ascii::matcher_backend::LOOKUP>('Q'));
static_assert(fastlex::ascii::isalnum<fastlex::ascii::matcher_backend::BITMAP>('Q'));
static_assert(fastlex::ascii::isalpha('Q'));
static_assert(fastlex::ascii::isdigit('3'));
static_assert(fastlex::ascii::isdigit<fastlex::ascii::matcher_backend::BITMAP>('3'));
static_assert(fastlex::ascii::isdigit<fastlex::ascii::matcher_backend::LOOKUP>('3'));
static_assert(!fastlex::ascii::isxdigit('_'));
static_assert(fastlex::ascii::tolower('A') == 'a');
static_assert(fastlex::ascii::toupper('z') == 'Z');
static_assert(fastlex::ascii::tolower('[') == '[');
static_assert(fastlex::ascii::toupper('`') == '`');
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isalnum(c) == expect_alnum(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isalpha(c) == expect_alpha(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isblank(c) == expect_blank(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::iscntrl(c) == expect_cntrl(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isdigit(c) == expect_digit(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isgraph(c) == expect_graph(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::islower(c) == expect_lower(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isprint(c) == expect_print(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::ispunct(c) == expect_punct(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isspace(c) == expect_space(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isupper(c) == expect_upper(c); }));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::isxdigit(c) == expect_xdigit(c);}));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::tolower(c) == expect_lowered(c);}));
static_assert(all_bytes([](unsigned char c) { return fastlex::ascii::toupper(c) == expect_uppered(c);}));
// clang-format on

} // namespace

TEST(FastlexAscii, MatchesAsciiCtypeClassesForEveryByte)
{
    for(int c = 0; c <= 255; ++c) {
        auto const byte = static_cast<unsigned char>(c);
        SCOPED_TRACE(c);
        EXPECT_EQ(fastlex::ascii::isalnum(byte), expect_alnum(c));
        EXPECT_EQ(fastlex::ascii::isalpha(byte), expect_alpha(c));
        EXPECT_EQ(fastlex::ascii::isblank(byte), expect_blank(c));
        EXPECT_EQ(fastlex::ascii::iscntrl(byte), expect_cntrl(c));
        EXPECT_EQ(fastlex::ascii::isdigit(byte), expect_digit(c));
        EXPECT_EQ(fastlex::ascii::isgraph(byte), expect_graph(c));
        EXPECT_EQ(fastlex::ascii::islower(byte), expect_lower(c));
        EXPECT_EQ(fastlex::ascii::isprint(byte), expect_print(c));
        EXPECT_EQ(fastlex::ascii::ispunct(byte), expect_punct(c));
        EXPECT_EQ(fastlex::ascii::isspace(byte), expect_space(c));
        EXPECT_EQ(fastlex::ascii::isupper(byte), expect_upper(c));
        EXPECT_EQ(fastlex::ascii::isxdigit(byte), expect_xdigit(c));
        EXPECT_EQ(fastlex::ascii::isascii(byte), c <= 0x7f);
    }
}

TEST(FastlexAscii, ConvertsCaseForEveryByte)
{
    for(int c = 0; c <= 255; ++c) {
        auto const byte = static_cast<unsigned char>(c);
        SCOPED_TRACE(c);
        EXPECT_EQ(fastlex::ascii::tolower(byte), expect_lowered(c));
        EXPECT_EQ(fastlex::ascii::toupper(byte), expect_uppered(c));
    }
}

TEST(FastlexAscii, KeepsCaseConversionBoundariesTight)
{
    EXPECT_EQ(fastlex::ascii::tolower('@'), '@');
    EXPECT_EQ(fastlex::ascii::tolower('A'), 'a');
    EXPECT_EQ(fastlex::ascii::tolower('Z'), 'z');
    EXPECT_EQ(fastlex::ascii::tolower('['), '[');

    EXPECT_EQ(fastlex::ascii::toupper('`'), '`');
    EXPECT_EQ(fastlex::ascii::toupper('a'), 'A');
    EXPECT_EQ(fastlex::ascii::toupper('z'), 'Z');
    EXPECT_EQ(fastlex::ascii::toupper('{'), '{');
}

TEST(FastlexAscii, SupportsCustomCompileTimeMatchers)
{
    constexpr auto separators = fastlex::ascii::custom(",:;|"sv);
    constexpr auto ident_start = fastlex::ascii::custom(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"sv);

    EXPECT_TRUE(separators(','));
    EXPECT_TRUE(separators('|'));
    EXPECT_FALSE(separators('x'));

    EXPECT_TRUE(ident_start('A'));
    EXPECT_TRUE(ident_start('z'));
    EXPECT_TRUE(ident_start('_'));
    EXPECT_FALSE(ident_start('7'));
}

TEST(FastlexAscii, SupportsBothCustomMatcherBackends)
{
    constexpr auto bitmap = fastlex::ascii::custom<fastlex::ascii::matcher_backend::BITMAP>(
        "0123456789_"sv);
    constexpr auto lookup = fastlex::ascii::custom<fastlex::ascii::matcher_backend::LOOKUP>(
        "0123456789_"sv);

    for(int c = 0; c <= 255; ++c) {
        auto const byte = static_cast<unsigned char>(c);
        auto const expected = expect_digit(c) || c == '_';
        SCOPED_TRACE(c);
        EXPECT_EQ(bitmap(byte), expected);
        EXPECT_EQ(lookup(byte), expected);
    }
}
