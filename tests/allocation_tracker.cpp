#include "allocation_tracker.h"
#include <algorithm>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace fc::alloc;

long fc::alloc::s_userAllocdBytes = 0;
long fc::alloc::s_allocdBytes = 0;
long fc::alloc::s_freedBytes = 0;
long fc::alloc::s_freeCount = 0;
long fc::alloc::s_maxBytesUsed = 0;
bool fc::alloc::s_debug = false;

void dbg(const char* type, long count) { if (fc::alloc::s_debug) std::cout << "[DBG ALLOC] " << type << ":" << count << std::endl; }

decltype(__malloc_hook) detail::s_oldMalloc = nullptr;
decltype(__realloc_hook) detail::s_oldRealloc = nullptr;
decltype(__free_hook) detail::s_oldFree = nullptr;
decltype(__memalign_hook) detail::s_oldMemalign = nullptr;

void* detail::malloc(size_t size, const void* caller)
{
    removeHooks();
    dbg(__PRETTY_FUNCTION__, size);
    auto ret = ::malloc(size);
    s_userAllocdBytes += size;
    s_allocdBytes += malloc_usable_size(ret);
    s_maxBytesUsed = std::max(s_maxBytesUsed, s_allocdBytes - s_freedBytes);
    addHooks();
    return ret;
}

void* detail::realloc(void* ptr, size_t size, const void* caller)
{
    removeHooks();
    dbg(__PRETTY_FUNCTION__, size);
    s_freedBytes += malloc_usable_size(ptr);
    s_freeCount++;
    auto ret = ::realloc(ptr, size);
    s_userAllocdBytes += size;
    s_allocdBytes += malloc_usable_size(ret);
    s_maxBytesUsed = std::max(s_maxBytesUsed, s_allocdBytes - s_freedBytes);
    addHooks();
    return ret;
}

void detail::free(void* ptr, const void* caller)
{
    removeHooks();
    dbg(__PRETTY_FUNCTION__, 0);
    s_freedBytes += malloc_usable_size(ptr);
    s_freeCount++;
    s_maxBytesUsed = std::max(s_maxBytesUsed, s_allocdBytes - s_freedBytes);
    ::free(ptr);
    addHooks();
}

void* detail::memalign(size_t alignment, size_t size, const void* caller)
{
    removeHooks();
    dbg(__PRETTY_FUNCTION__, size);
    s_userAllocdBytes += size;
    auto ret = ::memalign(alignment, size);
    s_allocdBytes += malloc_usable_size(ret);
    s_maxBytesUsed = std::max(s_maxBytesUsed, s_allocdBytes - s_freedBytes);
    addHooks();
    return ret;
}

void detail::addHooks()
{
    detail::s_oldMalloc = __malloc_hook;
    __malloc_hook = detail::malloc;

    detail::s_oldRealloc = __realloc_hook;
    __realloc_hook = detail::realloc;

    detail::s_oldFree = __free_hook;
    __free_hook = detail::free;

    detail::s_oldMemalign = __memalign_hook;
    __memalign_hook = detail::memalign;
}

void detail::removeHooks()
{
    __malloc_hook = detail::s_oldMalloc;
    __realloc_hook = detail::s_oldRealloc;
    __free_hook = detail::s_oldFree;
    __memalign_hook = detail::s_oldMemalign;
}

void fc::alloc::startTracking()
{
    s_userAllocdBytes = 0;
    s_allocdBytes = 0;
    s_freedBytes = 0;
    s_freeCount = 0;
    s_maxBytesUsed = 0;
    detail::addHooks();
}

long fc::alloc::endTracking()
{
    detail::removeHooks();
    return s_allocdBytes - s_freedBytes;
}

#pragma GCC diagnostic pop
