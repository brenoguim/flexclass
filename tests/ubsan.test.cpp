#include "allocation_tracker.h"

#include <catch.hpp>
#include <flexclass.hpp>

#include <cstring>

TEST_CASE( "Default array", "[sanitizer]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, std::string, int[]>
    {
        enum Members {Header, Data};
        using FLB::FLB;
    };

    auto r = Message::niw("SmallMsg", 1000);
    fc::deleet(r);
}

TEST_CASE( "Default array with non-trivial type", "[sanitizer]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, std::string, std::string[]>
    {
        enum Members {Header, Data};
        using FLB::FLB;
    };

    auto r = Message::niw("SmallMsg", 1000);
    int i = 0;
    for (auto& str : r->get<Message::Data>()) str.resize(i++);
    fc::deleet(r);
}

TEST_CASE( "SizedArray", "[sanitizer]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, std::string, fc::SizedArray<char>>
    {
        enum Members {Header, Data};
        using FLB::FLB;
    };

    auto r = Message::niw("SmallMsg", 1000);
    fc::deleet(r);
}

TEST_CASE( "Adjacent array", "[sanitizer]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, std::string, fc::AdjacentArray<char>>
    {
        enum Members {Header, Data};
        using FLB::FLB;
    };

    auto r = Message::niw("SmallMsg", 1000);
    fc::deleet(r);
}

TEST_CASE( "Adjacent array char->long to verify alignment", "[sanitizer]" )
{
    struct Message : public fc::FlexibleLayoutBase<Message, char, fc::AdjacentArray<long>>
    {
        enum Members {Header, Data};
        using FLB::FLB;
    };

    auto r = Message::niw('\0', 1000);
    for (int i = 0; i < 1000; ++i) r->begin<Message::Data>()[i] = 1;
    fc::deleet(r);
}

enum Members { E1, E2, E3 };
struct MyArray
{
    MyArray(fc::ArrayBuilder<long>&&) {}

    using type = long;
    using fc_array_kind = fc::unsized;
    enum { array_alignment = alignof(long) };

    template<class Derived>
    auto begin(const Derived* ptr) const
    {
        return fc::align<long>(ptr->template begin<E2>() + ptr->template get<E1>());
    }
};

TEST_CASE( "Adjacent array char->long to verify alignment with custom", "[sanitizer]" )
{
    using Message = fc::FlexibleLayoutClass<char, fc::AdjacentArray<char>, MyArray>;

    auto r = Message::niw(1, 1, 1);
    r->begin<E3>()[0] = 12983;
    fc::deleet(r);
}
