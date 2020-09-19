#include "allocation_tracker.h"

#include <catch.hpp>
#include <flexclass.hpp>

#include <cstring>

TEST_CASE( "Allocate and destroy", "[basic]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, std::string, char[]>
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
    struct Message : public fc::FlexibleLayoutBase<Message, std::string, fc::SizedArray<char>>
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

TEST_CASE( "Allocate and destroy but using an adjacent array", "[basic]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, std::string, fc::AdjacentArray<char>>
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


TEST_CASE( "Using adjacent arrays", "[basic]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, char, fc::AdjacentArray<long>>
    {
        enum Members {Header, Data};
        using FLC::FLC;
    };

    auto r = Message::niw('\0', 1000);

    *r->begin<Message::Data>() = 1;

    fc::deleet(r);
}

TEST_CASE( "Manipulate a FlexibleArrayClass directly", "[basic]" )
{
    using Message = fc::FlexibleLayoutClass<char, long[], bool>;

    auto r = Message::niw('\0', 100, false);
    r->get<0>() = 'a';
    r->begin<1>()[0] = 120391409823;
    r->get<2>() = true;

    CHECK(r->get<0>() == 'a');
    CHECK(r->begin<1>()[0] == 120391409823);
    CHECK(r->get<2>() == true);

    fc::deleet(r);
}
