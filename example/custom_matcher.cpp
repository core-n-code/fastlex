#include <fastlex/ascii.hpp>

#include <iostream>
#include <string_view>

using namespace std::literals;

auto main() -> int
{
    constexpr auto ident_start = fastlex::ascii::custom(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"sv);
    constexpr auto ident_continue = fastlex::ascii::custom<
        fastlex::ascii::matcher_backend::LOOKUP>(
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"sv);

    constexpr std::string_view input = "_token42!";

    for(std::size_t i = 0; i < input.size(); ++i) {
        auto const byte = static_cast<unsigned char>(input[i]);
        auto const accepted = (i == 0) ? ident_start(byte) : ident_continue(byte);

        std::cout << input[i] << " accepted=" << accepted << '\n';
    }
}

