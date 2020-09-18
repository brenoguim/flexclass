#include "allocation_tracker.h"

#include <catch.hpp>
#include <flexclass.hpp>

#include <cstring>

TEST_CASE( "Allocate and destroy", "[basic]" )
{
    struct Message : public fc::FlexibleLayoutClass<Message, std::string, char[]>
    {
        enum Members {Header, Data};
        using FLC::FLC;
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + sizeof(char*) + numChars*sizeof(char);

    auto r = fc::alloc::track([&] { return Message::niw("SmallMsg", numChars); });

    CHECK(fc::alloc::s_userAllocdBytes == expectedSize);
    CHECK(fc::alloc::s_freeCount == 0);

    fc::alloc::track([&] { fc::deleet(r); });

    CHECK(fc::alloc::s_allocdBytes == 0);
    CHECK(fc::alloc::s_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but forcing sized char", "[basic]" )
{
    struct Message : public fc::FlexibleLayoutClass<Message, std::string, fc::SizedArray<char>>
    {
        enum Members {Header, Data};
        using FLC::FLC;
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + 2*sizeof(char*) + numChars*sizeof(char);

    auto r = fc::alloc::track([&] { return Message::niw("SmallMsg", numChars); });

    CHECK(fc::alloc::s_userAllocdBytes == expectedSize);
    CHECK(fc::alloc::s_freeCount == 0);

    fc::alloc::track([&] { fc::deleet(r); });

    CHECK(fc::alloc::s_allocdBytes == 0);
    CHECK(fc::alloc::s_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but using a floating array", "[basic]" )
{
    struct Message : public fc::FlexibleLayoutClass<Message, std::string, fc::FloatingArray<char>>
    {
        enum Members {Header, Data};
        using FLC::FLC;
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + numChars*sizeof(char);

    auto r = fc::alloc::track([&] { return Message::niw("SmallMsg", numChars); });

    CHECK(fc::alloc::s_userAllocdBytes == expectedSize);
    CHECK(fc::alloc::s_freeCount == 0);

    fc::alloc::track([&] { fc::deleet(r); });

    CHECK(fc::alloc::s_allocdBytes == 0);
    CHECK(fc::alloc::s_freeCount == 1);
}
