#pragma once

#include "flexclass.hpp"

namespace fc
{

enum SizeTracking { track_size, dont_track_size };

template<class T> struct UnsizedArray
{
    UnsizedArray(ArrayBuilder<T>&& aph) : m_begin(aph.begin()) {}

    using type = T;
    using fc_array_kind = unsized;
    static constexpr auto array_alignment = alignof(T);

    template<class Derived>
    auto begin(const Derived* ptr) const { return m_begin; }

    auto begin() const { return m_begin; }

    T* const m_begin;
};

template<class T> struct SizedArray
{
    SizedArray(ArrayBuilder<T>&& aph) : m_begin(aph.begin()), m_end(aph.end()) {}

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

template<class T, int El = -1> struct UnsizedAdjacentArray
{
    UnsizedAdjacentArray(ArrayBuilder<T>&&) {}

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

template<class T, int El = -1> struct SizedAdjacentArray
{
    SizedAdjacentArray(ArrayBuilder<T>&& aph) : m_end(aph.end()) {}

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

template<class T, SizeTracking st = dont_track_size>
using Array = std::conditional_t<st == track_size, SizedArray<T>, UnsizedArray<T>>;

template<class T, int El = -1, SizeTracking st = dont_track_size>
using AdjacentArray = std::conditional_t<st == track_size, SizedAdjacentArray<T, El>, UnsizedAdjacentArray<T, El>>;

template<class T>
struct ArraySelector<T[]>
{
    using type = Array<T, std::is_trivially_destructible<T>::value ? dont_track_size : track_size>;
};

}
