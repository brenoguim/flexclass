#include <catch.hpp>
#include <flexclass.hpp>

#include <cstring>

TEST_CASE( "Empty class", "[Edge cases]" )
{
    struct Message
    {
        auto fc_handles() { return fc::make_tuple(); }
    };

    static_assert(sizeof(Message) == 1, "Size of empty message should be the minimum required by C++ - 1 byte");

    // Check that it's possible to instantiate and destroy
    fc::make_unique<Message>()();
}

TEST_CASE( "Just one array and no regular members", "[Edge cases]" )
{
    struct Message
    {
        fc::Array<short> zero;
        auto fc_handles() { return fc::make_tuple(&zero); }
    };

    auto m = fc::make_unique<Message>(1000)();

    static_assert(std::is_same_v<decltype(m->zero.begin()), short*>);

    for (int i = 0; i < 1000; ++i) m->zero.begin()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->zero.begin()[i] == i);
}

TEST_CASE( "Just one member and no array", "[Edge cases]" )
{
    struct Message
    {
        std::string str;
        auto fc_handles() { return fc::make_tuple(); }
    };

    auto initStr = "default initialized string for testing";
    auto m = fc::make_unique<Message>()(initStr);

    CHECK(m->str == initStr);

    auto str = "This is a rather long string to make sure it allocates";
    m->str = str;
    CHECK(m->str == str);
}

TEST_CASE( "Array<T> with trivial type", "[basic]" )
{
    struct Message
    {
        std::string str;
        fc::Array<int> ints;
        auto fc_handles() { return fc::make_tuple(&ints); }
    };

    auto m = fc::make_unique<Message>(1000)("SmallMsg");
    for (int i = 0; i < 1000; ++i) m->ints.begin()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->ints.begin()[i] == i);
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
    struct Message
    {
        std::string str;
        fc::AdjacentArray<int> m_ints;
        auto ints_begin() { return m_ints.begin(this); }
        auto fc_handles() { return fc::make_tuple(&m_ints); }
    };

    auto m = fc::make_unique<Message>(1000)("SmallMsg");
    for (int i = 0; i < 1000; ++i) m->ints_begin()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->ints_begin()[i] == i);
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
    struct Message
    {
        std::string str;
        fc::Range<long> longs;
        auto fc_handles() { return fc::make_tuple(&longs); }
    };
    auto m = fc::make_unique<Message>(1000)(initStr);

    for (int i = 0; i < 1000; ++i) m->longs.begin()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->longs.begin()[i] == i);

    // Default array with non-trivial type should also have the end of the array:
    std::size_t count = 0;
    auto *b = m->longs.begin(), *e = m->longs.end();
    for (; b != e; ++b)
    {
        ++count;
        *b = 42;
        CHECK(*b == 42);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->longs.begin(); b != e; ++b)
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

    struct Message
    {
        std::string str;
        fc::Range<std::string> strings;
        auto fc_handles() { return fc::make_tuple(&strings); }
    };
    auto m = fc::make_unique<Message>(1000)(initStr);

    for (int i = 0; i < 1000; ++i) m->strings.begin()[i] = initStr;
    for (int i = 0; i < 1000; ++i) CHECK(m->strings.begin()[i] == initStr);

    // Default array with non-trivial type should also have the end of the array:
    auto otherStr = "Another      initialized string for testing";
    std::size_t count = 0;
    std::string *b = m->strings.begin(), *e = m->strings.end();
    for (; b != e; ++b)
    {
        ++count;
        CHECK(*b == initStr);
        *b = otherStr;
        CHECK(*b == otherStr);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->strings.begin(); b != e; ++b)
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

    struct Message
    {
        std::string str;
        fc::AdjacentRange<long> m_longs;
        auto longs_begin() { return m_longs.begin(this); }
        auto longs_end() { return m_longs.end(this); }
        auto fc_handles() { return fc::make_tuple(&m_longs); }
    };

    auto m = fc::make_unique<Message>(1000)(initStr);

    for (int i = 0; i < 1000; ++i) m->longs_begin()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->longs_begin()[i] == i);

    // Default array with non-trivial type should also have the end of the array:
    std::size_t count = 0;
    auto *b = m->longs_begin(), *e = m->longs_end();
    for (; b != e; ++b)
    {
        ++count;
        *b = 42;
        CHECK(*b == 42);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->longs_begin(); b != e; ++b)
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

    struct Message
    {
        std::string str;
        fc::AdjacentRange<std::string> m_strings;
        auto strings_begin() { return m_strings.begin(this); }
        auto strings_end() { return m_strings.end(this); }
        auto fc_handles() { return fc::make_tuple(&m_strings); }
    };
    auto m = fc::make_unique<Message>(1000)(initStr);

    for (int i = 0; i < 1000; ++i) m->strings_begin()[i] = initStr;
    for (int i = 0; i < 1000; ++i) CHECK(m->strings_begin()[i] == initStr);

    // Default array with non-trivial type should also have the end of the array:
    auto otherStr = "Another      initialized string for testing";
    std::size_t count = 0;
    std::string *b = m->strings_begin(), *e = m->strings_end();
    for (; b != e; ++b)
    {
        ++count;
        CHECK(*b == initStr);
        *b = otherStr;
        CHECK(*b == otherStr);
    }
    CHECK(count == 1000);

    count = 0;
    for (b = m->strings_begin(); b != e; ++b)
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
    struct Message
    {
        char chr;
        fc::AdjacentArray<long> m_longs;
        auto longs_begin() { return m_longs.begin(this); }
        auto fc_handles() { return fc::make_tuple(&m_longs); }
    };

    auto m = fc::make_unique<Message>(1000)('\0');

    // Expect the first 'long' to be aligned 8 bytes after char
    CHECK((std::uintptr_t)&m->chr == (std::uintptr_t) m->longs_begin() - 8);

    for (int i = 0; i < 1000; ++i) m->longs_begin()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->longs_begin()[i] == i);
}

TEST_CASE( "char derived from AdjacentArray<long> to check that long* is aligned properly", "[alignment]" )
{
    struct Message : public fc::AdjacentArray<long>
    {
        fc::AdjacentArray<long>& longs() { return *this; }
        char chr;

        auto longs_begin() { return longs().begin(this); }
        auto fc_handles() { return fc::make_tuple(&longs()); }
    };

    auto m = fc::make_unique<Message>(1000)('\0');

    // Expect the first 'long' to be aligned 8 bytes after char
    CHECK((std::uintptr_t)&m->chr == (std::uintptr_t) m->longs_begin() - 8);

    for (int i = 0; i < 1000; ++i) m->longs_begin()[i] = i;
    for (int i = 0; i < 1000; ++i) CHECK(m->longs_begin()[i] == i);
}

TEST_CASE( "Adjacent array char->long to verify alignment with custom", "[alignment]" )
{
    struct Message
    {
        auto fc_handles() const { return fc::make_tuple(&B, &C); }
        auto fc_handles()       { return fc::make_tuple(&B, &C); }

        auto begin_B() { return B.begin(this); }
        auto end_B() { return B.end(this); }
        auto begin_C() { return C.begin(this); }

        char A;
        fc::AdjacentArray<long, 0> C;
        fc::AdjacentRange<char> B;
    };

    auto m = fc::make_unique<Message>(1, 1)((char)13);

    CHECK(m->A == 13);

    // The alignment here is suboptimal: [1byte char] [7byte padding] [8byte char*] [1byte char] [7byte padding] [8byte long]
    CHECK((std::uintptr_t)&m->A == (std::uintptr_t) m->begin_B() - 16);
    CHECK((std::uintptr_t)m->begin_B() == (std::uintptr_t) m->begin_C() - 8);
    CHECK((std::uintptr_t)m->end_B() == (std::uintptr_t) m->begin_C() - 7);

    m->A = '\0';
    m->begin_B()[0] = 42;
    m->begin_C()[0] = 84;

    CHECK(m->A == '\0');
    CHECK(*m->begin_B() == 42);
    CHECK(*m->begin_C() == 84);
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

    Thrower(const char*) : Thrower() {}
    Thrower(std::string) : Thrower() {}

    ~Thrower()
    {
        CHECK(s_throwerStack.back() == m_id);
        s_throwerStack.pop_back();
    }

    std::unique_ptr<int> m_someMemory;
    int m_id;
};

TEST_CASE( "All objects should be destroyed in the reverse order they were created", "[exception]" )
{
    resetToThrowAt(100000);
    auto initStr = "";

    struct Message
    {
        auto fc_handles() { return fc::make_tuple(&t1, &t2); }
        Thrower a;
        Thrower b;
        Thrower c;
        fc::Range<Thrower> t1;
        fc::Range<Thrower> t2;
    };

    auto m = fc::make_unique<Message>(100, 100)(initStr, initStr, initStr);
}

TEST_CASE( "Strong exception guarantees when member throws on constructor", "[exception]" )
{
    resetToThrowAt(2);

    struct Message
    {
        auto fc_handles() { return fc::make_tuple(); }
        Thrower a;
        Thrower b;
        Thrower c;
        Thrower d;
        Thrower e;
    };

    try
    {
        auto m = fc::make_unique<Message>()();
    }
    catch (std::runtime_error& err)
    {
        CHECK(std::string("2") == err.what());
    }
    checkReset();
}

TEST_CASE( "Strong exception guarantees when member array throws on constructor", "[exception]" )
{
    resetToThrowAt(15);

    struct Message
    {
        auto fc_handles() { return fc::make_tuple(&a1, &a2, &a3); }
        std::string a;
        fc::Range<Thrower> a1;
        fc::Range<Thrower> a2;
        fc::Range<Thrower> a3;
    };

    std::string initStr = "default initialized string for testing";
    try
    {
        auto m = fc::make_unique<Message>(10, 10, 10)(initStr);
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
    struct Message
    {
        auto fc_handles() { return fc::make_tuple(&a1, &a2, &a3); }
        std::string a;
        fc::Range<IncrementOnDestructor> a1;
        fc::Range<Thrower> a2;
        fc::Range<Thrower> a3;
    };

    resetToThrowAt(15);
    cnt = 0;

    std::string initStr = "default initialized string for testing";
    try
    {
        auto m = fc::make_unique<Message>(10, 10, 10)(initStr);
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
    struct Message
    {
        auto fc_handles() { return fc::make_tuple(&a1); }
        std::string a;
        fc::Range<Thrower> a1;
    };

    resetToThrowAt(10000);
    std::string initStr = "default initialized string for testing";
    auto m = fc::make_unique<Message>(10)(initStr);
}

TEST_CASE( "Arrays should be destroyed in the reverse order", "[exception]" )
{
    struct Message
    {
        auto fc_handles() { return fc::make_tuple(&a1, &a2); }
        std::string a;
        fc::Range<Thrower> a1;
        fc::Range<Thrower> a2;
    };

    resetToThrowAt(10000);
    std::string initStr = "default initialized string for testing";
    auto m = fc::make_unique<Message>(1,1)(initStr);
}

TEST_CASE( "Support initialization of arrays" , "[basic]" )
{
    struct Message
    {
        Message(int i) : a(i) {}
        auto fc_handles() { return fc::make_tuple(&a1); }
        int a;
        fc::Array<int> a1;
    };

    auto m = fc::make_unique<Message>(fc::arg(10))(10);

    std::vector<int> v;
    v.resize(10);
    std::iota(v.begin(), v.end(), 0);

    m = fc::make_unique<Message>(fc::arg(10, v.begin()))(10);
    CHECK( std::equal(v.begin(), v.end(), m->a1.begin()) );
}
