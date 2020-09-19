#pragma once

#include "flexclass.hpp"

namespace fc
{

template<class T> struct Array
{
    Array(ArrayBuilder<T>&& aph) : m_begin(aph.begin()) {}

    using type = T;
    using fc_array_kind = unsized;
    static constexpr auto array_alignment = alignof(T);

    template<class Derived>
    auto begin(const Derived* ptr) const { return m_begin; }

    auto begin() const { return m_begin; }

    T* const m_begin;
};

template<class T> struct Range
{
    Range(ArrayBuilder<T>&& aph) : m_begin(aph.begin()), m_end(aph.end()) {}

    using type = T;
    using fc_array_kind = sized;
    static constexpr auto array_alignment = alignof(T);

    template<class Derived>
    auto begin(const Derived* ptr) const { return m_begin; }

    template<class Derived>
    auto end(const Derived* ptr) const { return m_end; }

    auto begin() const { return m_begin; }
    auto end() const { return m_end; }

    T* const m_begin;
    T* const m_end;
};

template<class T, int El = -1> struct AdjacentArray
{
    AdjacentArray(ArrayBuilder<T>&&) {}

    using type = T;
    using fc_array_kind = unsized;
    static constexpr auto array_alignment = alignof(T);

    template<class Derived>
    auto begin(const Derived* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }
};

template<class T, int El = -1> struct AdjacentRange
{
    AdjacentRange(ArrayBuilder<T>&& aph) : m_end(aph.end()) {}

    using type = T;
    using fc_array_kind = sized;
    static constexpr auto array_alignment = alignof(T);

    template<class Derived>
    auto begin(const Derived* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }

    template<class Derived>
    auto end(const Derived* ptr) const { return m_end; }

    T* const m_end;
};

template<class T>
struct ArraySelector<T[]>
{
    using type =
        std::conditional_t<std::is_trivially_destructible<T>::value,
                           Array<T>, Range<T>>;
};

}
