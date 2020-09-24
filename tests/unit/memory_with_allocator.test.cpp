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
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<std::string, char[]>;

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + sizeof(char*) + numChars*sizeof(char);

    AllocTrack alloc;

    auto m = Message::make(fc::withAllocator, alloc, "SmallMsg", numChars);

    CHECK(alloc.m_allocd == expectedSize);
    CHECK(alloc.m_freeCount == 0);

    alloc.resetCounters();
    fc::destroy(m, alloc);

    CHECK(alloc.m_allocd == 0);
    CHECK(alloc.m_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but forcing sized char", "[allocator]" )
{
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<std::string, fc::Range<char>>;

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + 2*sizeof(char*) + numChars*sizeof(char);

    AllocTrack alloc;

    auto m = Message::make(fc::withAllocator, alloc, "SmallMsg", numChars);

    CHECK(alloc.m_allocd == expectedSize);
    CHECK(alloc.m_freeCount == 0);

    alloc.resetCounters();
    fc::destroy(m, alloc);

    CHECK(alloc.m_allocd == 0);
    CHECK(alloc.m_freeCount == 1);
}

TEST_CASE( "Allocate and destroy but using an adjacent array", "[allocator]" )
{
    enum Members {Header, Data};
    using Message = fc::FlexibleClass<std::string, fc::AdjacentArray<char>>;

    auto numChars = 1000;
    auto expectedSize = sizeof(std::string) + numChars*sizeof(char);

    AllocTrack alloc;

    auto r = Message::make(fc::withAllocator, alloc, "SmallMsg", numChars);

    CHECK(alloc.m_allocd == expectedSize);
    CHECK(alloc.m_freeCount == 0);

    alloc.resetCounters();
    fc::destroy(r, alloc);

    CHECK(alloc.m_allocd == 0);
    CHECK(alloc.m_freeCount == 1);
}

