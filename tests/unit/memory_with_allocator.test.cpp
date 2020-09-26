#include <catch.hpp>
#include <flexclass.hpp>

#include <cstring>
#include <unordered_map>

struct AllocTrack
{
    void* allocate(std::size_t sz)
    {
        m_allocd += sz;
        auto mem = ::operator new(sz);
        m_ptr2sz[(uintptr_t)mem] = sz;
        return mem;
    }

    void deallocate(void* ptr)
    {
        auto it = m_ptr2sz.find((uintptr_t)ptr);
        CHECK(it != m_ptr2sz.end());
        m_deallocd += it->second;
        m_freeCount++;
        m_ptr2sz.erase(it);
        ::operator delete(ptr);
    }

    void resetCounters()
    {
        m_allocd = m_deallocd = m_freeCount = 0;
    }

    std::size_t m_allocd {0};
    std::size_t m_deallocd {0};
    std::size_t m_freeCount {0};
    std::unordered_map<uintptr_t, std::size_t> m_ptr2sz;
};

TEST_CASE( "Allocate and destroy", "[allocator]" )
{
    struct Message
    {
        auto fc_handles() { return fc::make_tuple(&data); }
        std::string str;
        fc::Array<char> data;
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + sizeof(char*) + numChars*sizeof(char);

    AllocTrack alloc;

    auto m = fc::make<Message>(fc::withAllocator, alloc, numChars)("SmallMsg");

    CHECK(alloc.m_allocd == expectedSize);
    CHECK(alloc.m_freeCount == 0);

    alloc.resetCounters();
    fc::destroy(m, alloc);

    CHECK(alloc.m_allocd == 0);
    CHECK(alloc.m_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but forcing sized char", "[allocator]" )
{
    struct Message
    {
        auto fc_handles() { return fc::make_tuple(&data); }
        std::string str;
        fc::Range<char> data;
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + 2*sizeof(char*) + numChars*sizeof(char);

    AllocTrack alloc;

    auto m = fc::make<Message>(fc::withAllocator, alloc, numChars)("SmallMsg");

    CHECK(alloc.m_allocd == expectedSize);
    CHECK(alloc.m_freeCount == 0);

    alloc.resetCounters();
    fc::destroy(m, alloc);

    CHECK(alloc.m_allocd == 0);
    CHECK(alloc.m_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but using an adjacent array", "[allocator]" )
{
    struct Message : public fc::AdjacentArray<char>
    {
        auto fc_handles() { return fc::make_tuple(&data()); }
        std::string str;
        fc::AdjacentArray<char>& data() { return *this; }
    };

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + numChars*sizeof(char);

    AllocTrack alloc;

    auto r = fc::make<Message>(fc::withAllocator, alloc, numChars)("SmallMsg");

    CHECK(alloc.m_allocd == expectedSize);
    CHECK(alloc.m_freeCount == 0);

    alloc.resetCounters();
    fc::destroy(r, alloc);

    CHECK(alloc.m_allocd == 0);
    CHECK(alloc.m_freeCount == 1);
}

