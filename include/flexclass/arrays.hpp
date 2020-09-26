#ifndef FC_FLEXCLASS_ARRAYS_HPP
#define FC_FLEXCLASS_ARRAYS_HPP

#include "core.hpp"

/*! Contains builtin handle implementations for common applications
 *
 * A handle must derive from fc::Handle<T> and define the following
 * methods:
 *
 * // Called by the library to let the handle know where T objects
 * // were created
 *
 * void setLocation(T* begin, T* end);
 *
 * // Called by the library when the user requests the begin of the
 * // object sequence
 * // The base is passed as a parameter for handles that need to
 * // query it
 *
 * template <class Base>
 * auto begin(const Base*) const -> T*;
 *
 * [optional]
 * // Called by the library when the user requests the end of the
 * // object sequence
 * // The base is passed as a parameter for handles that need to
 * // query it
 *
 * template <class Base>
 * auto end(const Base*) const -> T*;
 *
 */

namespace fc
{

/*! Uses a pointer to store the location of the first T
 *  in the sequence.
 *  Does not know the size of the array.
 */
template <class T>
struct Array : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end) { m_begin = begin; }

    template <class Base = void>
    auto begin(const Base* ptr = nullptr) const
    {
        return m_begin;
    }

    //auto begin() const { return m_begin; }

    T* m_begin;
};

/*! Uses two pointers to store the location of the first T
 *  and last T of the sequence.
 */
template <class T>
struct Range : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end)
    {
        m_begin = begin;
        m_end = end;
    }

    template <class Base>
    auto begin(const Base* ptr) const
    {
        return m_begin;
    }

    template <class Base>
    auto end(const Base* ptr) const
    {
        return m_end;
    }

    auto begin() const { return m_begin; }
    auto end() const { return m_end; }

    T* m_begin;
    T* m_end;
};

/*! Uses another handle index to derivate the
 *  sequence position.
 *
 *  If El == -1, then it assumes the begin is
 *  adjacent to the Base.
 *
 *  Otherwise it takes the end() of the El handle
 *  and assumes that is where the T array begins
 */
template <class T, int El = -1>
struct AdjacentArray : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end) {}

    template <class Base>
    auto begin(const Base* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }
};
namespace v2 {
template <class T, int El = -1>
struct AdjacentArray : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end) {}

    template <class Base>
    auto begin(const Base* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else {
            auto e = ptr->fc_handles().template get<El>()->end(ptr);
            return aligner(e).template get<T>();
        }
    }
};
}

/*! Uses another handle index to derivate the
 *  sequence position.
 *
 *  If El == -1, then it assumes the begin is
 *  adjacent to the Base.
 *
 *  Otherwise it takes the end() of the El handle
 *  and assumes that is where the T array begins
 *
 *  Uses an extra T* to store the end of the array
 */
template <class T, int El = -1>
struct AdjacentRange : Handle<T>
{
    using Handle<T>::Handle;

    void setLocation(T* begin, T* end) { m_end = end; }

    template <class Base>
    auto begin(const Base* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }

    template <class Base>
    auto end(const Base* ptr) const
    {
        return m_end;
    }

    T* m_end;
};

} // namespace fc

#endif // FC_FLEXCLASS_ARRAYS_HPP
