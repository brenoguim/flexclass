#ifndef FC_FLEXCLASS_ARRAYS_HPP
#define FC_FLEXCLASS_ARRAYS_HPP

#include "core.hpp"

namespace fc
{
template <class T>
struct Array : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end) { m_begin = begin; }

    template <class Derived>
    auto begin(const Derived* ptr) const
    {
        return m_begin;
    }

    auto begin() const { return m_begin; }

    T* m_begin;
};

template <class T>
struct Range : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end)
    {
        m_begin = begin;
        m_end = end;
    }

    template <class Derived>
    auto begin(const Derived* ptr) const
    {
        return m_begin;
    }

    template <class Derived>
    auto end(const Derived* ptr) const
    {
        return m_end;
    }

    auto begin() const { return m_begin; }
    auto end() const { return m_end; }

    T* m_begin;
    T* m_end;
};

template <class T, int El = -1>
struct AdjacentArray : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end) {}

    template <class Derived>
    auto begin(const Derived* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }
};

template <class T, int El = -1>
struct AdjacentRange : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end) { m_end = end; }

    template <class Derived>
    auto begin(const Derived* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }

    template <class Derived>
    auto end(const Derived* ptr) const
    {
        return m_end;
    }

    T* m_end;
};

template <class T>
struct ArraySelector<T[]>
{
    using type = std::conditional_t<std::is_trivially_destructible<T>::value,
                                    Array<T>, Range<T>>;
};

} // namespace fc

#endif // FC_FLEXCLASS_ARRAYS_HPP
