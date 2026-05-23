#include <fastlex/ascii.hpp>

#include <iostream>
#include <string_view>

int main()
{
    constexpr std::string_view token = "Cafe_BEEF42";

    for (unsigned char c : token) {
        std::cout << c << " alpha=" << fastlex::ascii::isalpha(c)
                  << " digit=" << fastlex::ascii::isdigit(c)
                  << " hex=" << fastlex::ascii::isxdigit(c)
                  << " lower=" << fastlex::ascii::tolower(c)
                  << " upper=" << fastlex::ascii::toupper(c) << '\n';
    }
}
