#include <catch.hpp>
#include <flexclass.hpp>

#include <cstring>

TEST_CASE( "Empty class", "[Edge cases]" )
{
    using Message = fc::FlexibleClass<>;

    static_assert(sizeof(Message) == 1, "Size of empty message should be the minimum required by C++ - 1 byte");
    static_assert(Message::numMembers() == 0);

    // Check that it's possible to instantiate and destroy
    Message::make_unique();
}

TEST_CASE( "Just one array and no regular members", "[Edge cases]" )
{
    auto m = fc::FlexibleClass<short[]>::make_unique(1000);

    CHECK(m->numMembers() == 1);
    static_assert(std::is_same_v<decltype(m->begin<0>()), short*>);

    for (int i = 0; i < 1000; ++i) m->begin<0>()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<0>()[i] == i);
}

TEST_CASE( "Just one member and no array", "[Edge cases]" )
{
    auto initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string>::make_unique(initStr);

    CHECK(m->numMembers() == 1);
    static_assert(std::is_same_v<decltype(m->get<0>()), std::string&>);

    CHECK(m->get<0>() == initStr);

    auto str = "This is a rather long string to make sure it allocates";
    m->get<0>() = str;
    CHECK(m->get<0>() == str);
}

TEST_CASE( "Default array", "[sanitizer]" )
{
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<std::string, int[]>;

    auto r = Message::make("SmallMsg", 1000);
    fc::destroy(r);
}

TEST_CASE( "Default array with non-trivial type", "[sanitizer]" )
{
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<std::string, std::string[]>;

    auto r = Message::make("SmallMsg", 1000);
    int i = 0;
    for (auto& str : r->get<Data>()) str.resize(i++);
    fc::destroy(r);
}

TEST_CASE( "SizedArray", "[sanitizer]" )
{
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<std::string, fc::Range<int>>;

    auto r = Message::make("SmallMsg", 1000);
    fc::destroy(r);
}

TEST_CASE( "Adjacent array", "[sanitizer]" )
{
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<std::string, int[]>;

    auto r = Message::make("SmallMsg", 1000);
    fc::destroy(r);
}

TEST_CASE( "Adjacent array char->long to verify alignment", "[sanitizer]" )
{
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<char, long[]>;

    auto r = Message::make('\0', 1000);
    for (int i = 0; i < 1000; ++i) r->begin<Data>()[i] = 1;
    fc::destroy(r);
}

namespace t1 {
enum Members { E1, E2, E3 };
struct MyArray
{
    template<class T> MyArray(T&&) {}
    void setLocation(long*, long*) {}

    using type = long;
    using fc_handle = fc::handle::array;
    static constexpr auto array_alignment = alignof(type);

    template<class Derived>
    auto begin(const Derived* ptr) const
    {
        auto e2begin = ptr->template begin<E2>();
        auto e2len = ptr->template get<E1>();
        return fc::aligner(e2begin, e2len).template get<long>();
    }
};

TEST_CASE( "Adjacent array char->long to verify alignment with custom", "[sanitizer]" )
{
    using Message = fc::FlexibleClass<char, fc::AdjacentArray<char>, MyArray>;

    auto r = Message::make(1, 1, 1);
    r->begin<E3>()[0] = 12983;
    fc::destroy(r);
}
}


namespace t2 {
enum Members { E1, E2, E3 };
struct MyArray1
{
    template<class T> MyArray1(T&&) {}
    void setLocation(char*, char*) {}

    using type = char;
    using fc_handle = fc::handle::range;
    static constexpr auto array_alignment = alignof(type);

    template<class Derived>
    auto begin(const Derived* ptr) const
    {
        return fc::aligner(ptr,1).template get<char>();
    }

    template<class Derived>
    auto end(const Derived* ptr) const
    {
        return begin(ptr) + ptr->template get<E1>();
    }
};

TEST_CASE( "Test co-dependent array", "[sanitizer]" )
{
    using Message = fc::FlexibleClass<char, MyArray1, fc::AdjacentArray<long, E2>>;

    auto r = Message::make(1, 1, 1);
    r->begin<E3>()[0] = 12983;
    fc::destroy(r);
}
}

