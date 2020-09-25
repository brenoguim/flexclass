#ifndef FLEXCLASS_MEMORY_HPP
#define FLEXCLASS_MEMORY_HPP
namespace fc
{

/* Default allocator that calls operator new/delete
 */
struct NewDeleteAllocator
{
    void* allocate(std::size_t sz) { return ::operator new(sz); }
    void deallocate(void* ptr) { ::operator delete(ptr); }
};

template <class T>
struct ArrayDeleter
{
    ArrayDeleter(T* begin) : m_begin(begin), m_end(begin) {}

    ~ArrayDeleter() { reverseDestroy(m_begin, m_end); }

    void setEnd(T* end) { m_end = end; }
    void release() { m_begin = m_end = nullptr; }

    T *m_begin{nullptr}, *m_end{nullptr};
};

inline void* incr(void* in, std::size_t len) { return static_cast<std::byte*>(in) + len; }

constexpr std::uintptr_t findNextAlignedPosition(std::size_t pos, std::size_t desiredAlignment, std::size_t currentAlignment = 1)
{
    return (pos - 1u + desiredAlignment) & -desiredAlignment;
}

template<class T>
constexpr std::uintptr_t findNextAlignedPosition(T* ptr, std::size_t desiredAlignment, std::size_t currentAlignment = alignof(T))
{
    return findNextAlignedPosition(static_cast<std::uintptr_t>(ptr), desiredAlignment, currentAlignment);
}

template <class T, class U>
inline auto align(U* u)
{
    return reinterpret_cast<T*>(findNextAlignedPosition(u, alignof(T)));
}

template <class T>
struct aligner_impl
{
    aligner_impl& advance(std::size_t len)
    {
        ptr += len;
        return *this;
    };
    template <class U>
    auto cast()
    {
        return aligner_impl<U>{align<U>(ptr)};
    }
    template <class U>
    auto get()
    {
        return cast<U>().ptr;
    }
    T* ptr;
};

template <class T>
auto aligner(T* t)
{
    return aligner_impl<T>{t};
}
template <class T>
auto aligner(const T* t)
{
    return aligner_impl<T>{const_cast<T*>(t)};
}
template <class T>
auto aligner(T* t, std::size_t len)
{
    return aligner_impl<T>{t}.advance(len);
}
template <class T>
auto aligner(const T* t, std::size_t len)
{
    return aligner_impl<T>{const_cast<T*>(t)}.advance(len);
}

template <class T, class Deleter>
struct unique_ptr : private Deleter
{
    explicit unique_ptr(T* t) : m_t(t) {}
    template <class DeleterArg>
    unique_ptr(T* t, DeleterArg&& darg) : Deleter(std::forward<DeleterArg>(darg)), m_t(t)
    {
    }
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr(unique_ptr&& other)
        : Deleter(std::move(other.get_deleter())), m_t(std::exchange(other.m_t, nullptr))
    {
    }
    unique_ptr& operator=(const unique_ptr&) = delete;
    unique_ptr& operator=(unique_ptr&& other)
    {
        using std::swap;
        swap(get_deleter(), other.get_deleter());
        swap(m_t, other.m_t);
        return *this;
    }
    ~unique_ptr()
    {
        if (m_t)
            get_deleter()(m_t);
    }
    void release() { m_t = nullptr; }
    Deleter& get_deleter() { return *this; }
    T* operator->() { return m_t; }
    T* operator->() const { return m_t; }
    T* get() { return m_t; }
    T* get() const { return m_t; }
    T* m_t;
};

/*! Two step deleter
 *  It manages a void* that is deallocated with the allocator.
 *  The user can then set "m_objectCreated" to true, and it will
 *  also call the destructor of such type.
 */
template <class T, class Alloc>
struct DeleteFn
{
    DeleteFn(Alloc& alloc) : m_alloc(&alloc) {}
    void operator()(void* ptr) const
    {
        if (m_objectCreated)
            static_cast<T*>(ptr)->~T();
        m_alloc->deallocate(ptr);
    }
    Alloc* m_alloc;
    bool m_objectCreated{false};
};

} // namespace fc
#endif
