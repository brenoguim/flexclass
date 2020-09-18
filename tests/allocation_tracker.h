#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <malloc.h>

namespace fc {
namespace alloc {

extern long s_userAllocdBytes;
extern long s_allocdBytes;
extern long s_freedBytes;
extern long s_freeCount;
extern long s_maxBytesUsed;
extern bool s_debug;

void startTracking();
long endTracking();

template<class Fn>
struct finally
{
    finally(Fn fn) : m_fn(fn) {}
    ~finally() { m_fn(); }
    Fn m_fn;
};

template<class Fn>
decltype(auto) track(const Fn& fn)
{
    startTracking();
    finally f(endTracking);
    return fn();
}

namespace detail {

extern decltype(__malloc_hook) s_oldMalloc;
extern decltype(__realloc_hook) s_oldRealloc;
extern decltype(__free_hook) s_oldFree;
extern decltype(__memalign_hook) s_oldMemalign;

void addHooks();
void removeHooks();

void* malloc(size_t size, const void* caller);
void* realloc(void* ptr, size_t size, const void* caller);
void free(void* ptr, const void* caller);
void* memalign(size_t alignment, size_t size, const void* caller);

} // namespace detail

} // namespace alloc
} // namespace fc

#pragma GCC diagnostic pop
