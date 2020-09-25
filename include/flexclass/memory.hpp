#ifndef FLEXCLASS_MEMORY_HPP
#define FLEXCLASS_MEMORY_HPP
namespace fc
{

template <class T>
struct ArrayDeleter
{
    ArrayDeleter(T* begin) : m_begin(begin), m_end(begin) {}

    ~ArrayDeleter() { reverse_destroy(m_begin, m_end); }

    void setEnd(T* end) { m_end = end; }
    void release() { m_begin = m_end = nullptr; }

    T *m_begin{nullptr}, *m_end{nullptr};
};

inline void* incr(void* in, std::size_t len)
{
    return static_cast<std::byte*>(in) + len;
}

template <class T, class U>
inline auto align(U* u)
{
    void* ptr = u;
    if (alignof(U) >= alignof(T))
        return static_cast<T*>(ptr);

    std::size_t space = std::numeric_limits<std::size_t>::max();
    return static_cast<T*>(std::align(alignof(T), 0, ptr, space));
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
    unique_ptr(T* t, DeleterArg&& darg)
        : Deleter(std::forward<DeleterArg>(darg)), m_t(t)
    {
    }
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr(unique_ptr&& other)
        : Deleter(std::move(other.get_deleter())),
          m_t(std::exchange(other.m_t, nullptr))
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

} // namespace fc
#endif
