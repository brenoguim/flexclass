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

TEST_CASE( "Default array with trivial type", "[basic]" )
{
    auto m = fc::FlexibleClass<std::string, int[]>::make_unique("SmallMsg", 1000);
    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == i);
}

TEST_CASE( "Default array with non-trivial type", "[basic]" )
{
    auto initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string, std::string[]>::make_unique(initStr, 1000);

    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = initStr;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == initStr);

    // Default array with non-trivial type should also have the end of the array:
    auto otherStr = "Another      initialized string for testing";
    std::size_t count = 0;
    std::string *b = m->begin<1>(), *e = m->end<1>();
    for (; b != e; ++b)
    {
        ++count;
        CHECK(*b == initStr);
        *b = otherStr;
        CHECK(*b == otherStr);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->begin<1>(); b != e; ++b)
    {
        ++count;
        CHECK(*b == otherStr);
        *b = initStr;
        CHECK(*b == initStr);
    }
    CHECK(count == 1000);
}

TEST_CASE( "Array<T> with trivial type", "[basic]" )
{
    auto m = fc::FlexibleClass<std::string, fc::Array<int>>::make_unique("SmallMsg", 1000);
    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == i);
}

TEST_CASE( "Array<T> array with non-trivial type", "[basic]" )
{
    // This should not compile. The destructor of FlexibleClass needs to call the destructor of std::string
    //     but for that it needs begin/end. Since Array does not provide "end", the compiler should fail.
    //     Not sure how to test that programatically
    //auto m = fc::FlexibleClass<std::string, fc::Array<std::string>>::make_unique("", 1000);
}

TEST_CASE( "AdjacentArray<T> with trivial type", "[basic]" )
{
    auto m = fc::FlexibleClass<std::string, fc::AdjacentArray<int>>::make_unique("SmallMsg", 1000);
    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == i);
}

TEST_CASE( "AdjacentArray<T> array with non-trivial type", "[basic]" )
{
    // This should not compile. The destructor of FlexibleClass needs to call the destructor of std::string
    //     but for that it needs begin/end. Since Array does not provide "end", the compiler should fail.
    //     Not sure how to test that programatically
    //auto m = fc::FlexibleClass<std::string, fc::Array<std::string>>::make_unique("", 1000);
}

TEST_CASE( "Range<T> with trivial type", "[basic]" )
{
    auto initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string, fc::Range<long>>::make_unique(initStr, 1000);

    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == i);

    // Default array with non-trivial type should also have the end of the array:
    std::size_t count = 0;
    auto *b = m->begin<1>(), *e = m->end<1>();
    for (; b != e; ++b)
    {
        ++count;
        *b = 42;
        CHECK(*b == 42);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->begin<1>(); b != e; ++b)
    {
        ++count;
        CHECK(*b == 42);
        *b = 1234567890;
        CHECK(*b == 1234567890);
    }
    CHECK(count == 1000);
}

TEST_CASE( "Range<T> array with non-trivial type", "[basic]" )
{
    auto initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string, fc::Range<std::string>>::make_unique(initStr, 1000);

    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = initStr;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == initStr);

    // Default array with non-trivial type should also have the end of the array:
    auto otherStr = "Another      initialized string for testing";
    std::size_t count = 0;
    std::string *b = m->begin<1>(), *e = m->end<1>();
    for (; b != e; ++b)
    {
        ++count;
        CHECK(*b == initStr);
        *b = otherStr;
        CHECK(*b == otherStr);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->begin<1>(); b != e; ++b)
    {
        ++count;
        CHECK(*b == otherStr);
        *b = initStr;
        CHECK(*b == initStr);
    }
    CHECK(count == 1000);
}

TEST_CASE( "AdjacentRange<T> with trivial type", "[basic]" )
{
    auto initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string, fc::AdjacentRange<long>>::make_unique(initStr, 1000);

    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == i);

    // Default array with non-trivial type should also have the end of the array:
    std::size_t count = 0;
    auto *b = m->begin<1>(), *e = m->end<1>();
    for (; b != e; ++b)
    {
        ++count;
        *b = 42;
        CHECK(*b == 42);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->begin<1>(); b != e; ++b)
    {
        ++count;
        CHECK(*b == 42);
        *b = 1234567890;
        CHECK(*b == 1234567890);
    }
    CHECK(count == 1000);
}

TEST_CASE( "AdjacentRange<T> array with non-trivial type", "[basic]" )
{
    auto initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string, fc::AdjacentRange<std::string>>::make_unique(initStr, 1000);

    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = initStr;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == initStr);

    // Default array with non-trivial type should also have the end of the array:
    auto otherStr = "Another      initialized string for testing";
    std::size_t count = 0;
    std::string *b = m->begin<1>(), *e = m->end<1>();
    for (; b != e; ++b)
    {
        ++count;
        CHECK(*b == initStr);
        *b = otherStr;
        CHECK(*b == otherStr);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->begin<1>(); b != e; ++b)
    {
        ++count;
        CHECK(*b == otherStr);
        *b = initStr;
        CHECK(*b == initStr);
    }
    CHECK(count == 1000);
}

TEST_CASE( "char followed by AdjacentArray<long> to check that long* is aligned properly", "[alignment]" )
{
    auto m = fc::FlexibleClass<char, fc::AdjacentArray<long>>::make_unique('\0', 1000);

    // Expect the first 'long' to be aligned 8 bytes after char
    CHECK((std::uintptr_t)&m->get<0>() == (std::uintptr_t) m->begin<1>() - 8);

    for (int i = 0; i < 1000; ++i) m->begin<1>()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->begin<1>()[i] == i);
}

TEST_CASE( "Adjacent array char->long to verify alignment with custom", "[alignment]" )
{
    enum Members {A, B, C};
    auto m = fc::FlexibleClass<char, fc::AdjacentRange<char>, fc::AdjacentArray<long, 1>>::make_unique(13, 1, 1);

    CHECK(m->get<A>() == 13);

    // The alignment here is suboptimal: [1byte char] [7byte padding] [8byte char*] [1byte char] [7byte padding] [8byte long]
    CHECK((std::uintptr_t)&m->get<A>() == (std::uintptr_t) m->begin<B>() - 16);
    CHECK((std::uintptr_t)m->begin<B>() == (std::uintptr_t) m->begin<C>() - 8);
    CHECK((std::uintptr_t)m->end<B>() == (std::uintptr_t) m->begin<C>() - 7);

    m->get<A>() = '\0';
    m->begin<B>()[0] = 42;
    m->begin<C>()[0] = 84;

    CHECK(m->get<A>() == '\0');
    CHECK(*m->begin<B>() == 42);
    CHECK(*m->begin<C>() == 84);
}


int s_throwerId = 0;
std::vector<int> s_throwerStack;
int s_throwAtId = 0;

void resetToThrowAt(int i)
{
    s_throwerId = 0;
    s_throwerStack.clear();
    s_throwAtId = i;
}

void checkReset()
{
    CHECK(s_throwerStack.empty());
}

struct Thrower
{
    Thrower()
    {
        m_id = s_throwerId;

        if (m_id == s_throwAtId)
            throw std::runtime_error(std::to_string(m_id));

        m_someMemory = std::make_unique<int>(13);

        s_throwerStack.push_back(s_throwerId++);
    }

    Thrower(std::string) : Thrower() {}

    ~Thrower()
    {
        CHECK(s_throwerStack.back() == m_id);
        s_throwerStack.pop_back();
    }

    std::unique_ptr<int> m_someMemory;
    int m_id;
};

TEST_CASE( "Strong exception guarantees when member throws on constructor", "[exception]" )
{
    resetToThrowAt(0);

    std::string initStr = "default initialized string for testing";
    try
    {
        auto m = fc::FlexibleClass<std::string, Thrower>::make_unique(initStr, initStr);
    }
    catch (std::runtime_error& err)
    {
        CHECK(std::string("0") == err.what());
    }
    checkReset();
}

TEST_CASE( "Strong exception guarantees when member array throws on constructor", "[exception]" )
{
    resetToThrowAt(15);

    std::string initStr = "default initialized string for testing";
    try
    {
        auto m = fc::FlexibleClass<std::string, Thrower[], Thrower[], Thrower[]>::make_unique(initStr, 10, 10, 10);
    }
    catch (std::runtime_error& err)
    {
        CHECK(std::string("15") == err.what());
    }
    checkReset();
}

namespace {
    int cnt = 0;
    struct IncrementOnDestructor
    {
        ~IncrementOnDestructor() { cnt++; }
    };
}

TEST_CASE( "Strong exception guarantees when member array throws on constructor, and other arrays have noexcept constructors", "[exception]" )
{
    resetToThrowAt(15);
    cnt = 0;

    std::string initStr = "default initialized string for testing";
    try
    {
        auto m = fc::FlexibleClass<std::string, IncrementOnDestructor[], Thrower[], Thrower[]>::make_unique(initStr, 10, 10, 10);
    }
    catch (std::runtime_error& err)
    {
        CHECK(std::string("15") == err.what());
    }
    CHECK(cnt == 10);
    checkReset();
}

TEST_CASE( "Array elements should be destroyed in the reverse order of creation within the same array", "[exception]" )
{
    resetToThrowAt(10000);
    std::string initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string, Thrower[]>::make_unique(initStr, 10);
}

TEST_CASE( "Arrays should be destroyed in the reverse order", "[exception]" )
{
    resetToThrowAt(10000);
    std::string initStr = "default initialized string for testing";
    auto m = fc::FlexibleClass<std::string, Thrower[], Thrower[]>::make_unique(initStr, 1, 1);
}
