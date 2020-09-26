#include "allocation_tracker.h"

#include <catch.hpp>
#include <flexclass.hpp>

#include <cstring>
#include <string>

TEST_CASE( "Allocate and destroy", "[memory]" )
{
    struct Message
    {
        auto fc_handles() { return fc::v2::make_tuple(&data); }
        std::string header;
        fc::Array<char> data;
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + sizeof(char*) + numChars*sizeof(char);

    auto r = fc::alloc::track([&] { return fc::v2::make<Message>(numChars)("SmallMsg"); });

    CHECK(fc::alloc::s_userAllocdBytes == expectedSize);
    CHECK(fc::alloc::s_freeCount == 0);

    fc::alloc::track([&] { fc::v2::destroy(r); });

    CHECK(fc::alloc::s_allocdBytes == 0);
    CHECK(fc::alloc::s_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but forcing sized char", "[memory]" )
{
    struct Message
    {
        auto fc_handles() { return fc::v2::make_tuple(&data); }
        std::string header;
        fc::Range<char> data;
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + 2*sizeof(char*) + numChars*sizeof(char);

    auto r = fc::alloc::track([&] { return fc::v2::make<Message>(numChars)("SmallMsg"); });

    CHECK(fc::alloc::s_userAllocdBytes == expectedSize);
    CHECK(fc::alloc::s_freeCount == 0);

    fc::alloc::track([&] { fc::v2::destroy(r); });

    CHECK(fc::alloc::s_allocdBytes == 0);
    CHECK(fc::alloc::s_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but using an adjacent array", "[memory]" )
{
    struct Message : fc::AdjacentArray<char>
    {
        auto fc_handles() { return fc::v2::make_tuple(&data()); }
        std::string header;
        fc::AdjacentArray<char>& data() { return *this; }
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + numChars*sizeof(char);

    auto r = fc::alloc::track([&] { return fc::v2::make<Message>(numChars)("SmallMsg"); });

    CHECK(fc::alloc::s_userAllocdBytes == expectedSize);
    CHECK(fc::alloc::s_freeCount == 0);

    fc::alloc::track([&] { fc::v2::destroy(r); });

    CHECK(fc::alloc::s_allocdBytes == 0);
    CHECK(fc::alloc::s_freeCount == 1);
}
