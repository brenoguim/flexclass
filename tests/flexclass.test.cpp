#include "catch.hpp"
#include <flexclass.hpp>

#include <cstring>

TEST_CASE( "Allocate and destroy", "[basic]" )
{
    struct Message : public fc::FlexibleLayoutClass<Message, std::string, fc::SizedArray<char>>
    {
        enum Members {Header, Data};

        using FLC::FLC;

        static auto* niw(std::string header)
        {
            auto r = FLC::niw(std::move(header), 1000);
            std::strcpy(r->get<Data>().begin(), "This is the default message!");
            return r;
        }
    };

    auto r = Message::niw("Header");
    fc::deleet(r);
}
