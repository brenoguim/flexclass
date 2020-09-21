#include <catch.hpp>
#include <flexclass.hpp>

struct empty_struct {};

TEST_CASE( "Test tuple", "[tuple]" )
{
    static_assert(sizeof(fc::tuple<char>) == 1);
    static_assert(sizeof(fc::tuple<char, char>) == 2);
    static_assert(sizeof(fc::tuple<char, char*>) == 16);
    static_assert(sizeof(std::tuple<char*, empty_struct>) == 8);
    static_assert(sizeof(std::tuple<empty_struct, char*>) == 8);

    static_assert(sizeof(std::tuple<empty_struct, char*, empty_struct>) == 16);

    static_assert(sizeof(fc::tuple<char*, empty_struct>) == 8);
    static_assert(sizeof(fc::tuple<empty_struct, char*>) == 8);
    static_assert(sizeof(fc::tuple<empty_struct, char*, empty_struct>) == 8);
}
