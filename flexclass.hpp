// MIT License
// 
// Copyright (c) 2020 Breno Rodrigues Guimarães
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef FC_FLEXCLASS_FLEXCLASS_HPP
#define FC_FLEXCLASS_FLEXCLASS_HPP
#ifndef FC_FLEXCLASS_ALGORITHM_HPP
#define FC_FLEXCLASS_ALGORITHM_HPP
#include <initializer_list>
namespace fc
{
/*! Calls the destructor on all elements in the range
 *     from "begin" up to "end" in reverse order
 */
template <class T>
void reverseDestroy(T* begin, T* end)
{
    while (begin != end)
    {
        end--;
        end->~T();
    }
}
} // namespace fc
#endif
#ifndef FC_FLEXCLASS_ARRAYS_HPP
#define FC_FLEXCLASS_ARRAYS_HPP
#ifndef FC_FLEXCLASS_CORE_HPP
#define FC_FLEXCLASS_CORE_HPP
#ifndef FLEXCLASS_MEMORY_HPP
#define FLEXCLASS_MEMORY_HPP
#include <cstdint>
#include <utility>
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
constexpr std::uintptr_t findNextAlignedPosition(std::size_t pos, std::size_t desiredAlignment,
                                                 std::size_t currentAlignment = 1)
{
    return (pos - 1u + desiredAlignment) & -desiredAlignment;
}
template <class T>
constexpr std::uintptr_t findNextAlignedPosition(T* ptr, std::size_t desiredAlignment,
                                                 std::size_t currentAlignment = alignof(T))
{
    return findNextAlignedPosition(reinterpret_cast<std::uintptr_t>(ptr), desiredAlignment,
                                   currentAlignment);
}
template <class T, class U>
constexpr auto align(U* u)
{
    if constexpr (alignof(U) >= alignof(T))
        return reinterpret_cast<T*>(u);
    else
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
struct unique_ptr_impl : private Deleter
{
    explicit unique_ptr_impl(T* t) : m_t(t) {}
    template <class DeleterArg>
    unique_ptr_impl(T* t, DeleterArg&& darg) : Deleter(std::forward<DeleterArg>(darg)), m_t(t)
    {
    }
    unique_ptr_impl(const unique_ptr_impl&) = delete;
    unique_ptr_impl(unique_ptr_impl&& other)
        : Deleter(std::move(other.get_deleter())), m_t(std::exchange(other.m_t, nullptr))
    {
    }
    unique_ptr_impl& operator=(const unique_ptr_impl&) = delete;
    unique_ptr_impl& operator=(unique_ptr_impl&& other)
    {
        using std::swap;
        swap(get_deleter(), other.get_deleter());
        swap(m_t, other.m_t);
        return *this;
    }
    ~unique_ptr_impl()
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
#ifndef FC_FLEXCLASS_TUPLE_HPP
#define FC_FLEXCLASS_TUPLE_HPP
#include <cstddef>
#include <cstdint>
namespace fc
{
template <class T1, class... Ts, class Buf, class Arg1, class... Args>
void callConstructors(int& constructed, Buf* buf, Arg1&& arg1, Args&&... args)
{
    if constexpr (std::is_aggregate_v<T1>)
        new (buf) T1{std::forward<Arg1>(arg1)};
    else
        new (buf) T1(std::forward<Arg1>(arg1));
    constructed++;
    if constexpr (sizeof...(Ts) > 0)
        callConstructors<Ts...>(constructed, buf + 1, std::forward<Args>(args)...);
}
template <class T1, class... Ts, class Buf>
void callDefaultConstructors(int& constructed, Buf* buf)
{
    new (buf) T1;
    constructed++;
    if constexpr (sizeof...(Ts) > 0)
        callDefaultConstructors<Ts...>(constructed, buf + 1);
}
template <int Id, class T1, class... Ts, class Buf>
void callDestructors(int constructed, Buf* buf)
{
    if constexpr (sizeof...(Ts) > 0)
        callDestructors<Id + 1, Ts...>(constructed, buf + 1);
    if (Id < constructed)
        reinterpret_cast<T1*>(buf)->~T1();
}
template <class... T>
struct MemInfo;
template <class F, class... T>
struct MemInfo<F, T...> : public MemInfo<T...>
{
    using Base = MemInfo<T...>;
    static constexpr auto Size = sizeof(F) > Base::Size ? sizeof(F) : Base::Size;
    static constexpr auto Align = alignof(F) > Base::Align ? alignof(F) : Base::Align;
};
template <>
struct MemInfo<>
{
    static constexpr auto Size = 1;
    static constexpr auto Align = 1;
};
template <int I, class T, class... Ts>
constexpr auto typePtrAtIdx()
{
    if constexpr (I == 0)
        return static_cast<T*>(nullptr);
    else
        return typePtrAtIdx<I - 1, Ts...>();
}
template <int I, class... Ts>
using TypeAtIdx = std::remove_pointer_t<decltype(typePtrAtIdx<I, Ts...>())>;
template <class... T>
struct tuple
{
    static constexpr auto Size = sizeof...(T);
    tuple()
    {
        if constexpr (Size > 0)
        {
            int constructed = 0;
            try
            {
                callDefaultConstructors<T...>(constructed, elements);
            }
            catch (...)
            {
                callDestructors<0, T...>(constructed, elements);
                throw;
            }
        }
    }
    template <class... Args>
    tuple(Args&&... args)
    {
        if constexpr (Size > 0)
        {
            int constructed = 0;
            try
            {
                callConstructors<T...>(constructed, elements, std::forward<Args>(args)...);
            }
            catch (...)
            {
                callDestructors<0, T...>(constructed, elements);
                throw;
            }
        }
    }
    ~tuple()
    {
        if constexpr (Size > 0)
            callDestructors<0, T...>(Size, elements);
    }
    tuple(const tuple&) = delete;
    tuple(tuple&&) = delete;
    tuple& operator=(const tuple&) = delete;
    tuple& operator=(tuple&&) = delete;
    template <int I>
    auto& get()
    {
        return reinterpret_cast<TypeAtIdx<I, T...>&>(elements[I]);
    }
    template <int I>
    auto& get() const
    {
        return reinterpret_cast<const TypeAtIdx<I, T...>&>(elements[I]);
    }
    using MI = MemInfo<T...>;
    std::aligned_storage_t<MI::Size, MI::Align> elements[Size];
};
namespace detail
{
template <int Idx, class T, class Fn>
void callWithIdx(const T& t, Fn&& f)
{
    f(t.template get<Idx>(), std::integral_constant<int, Idx>());
}
template <int Idx, class T, class Fn>
void callWithIdx(T& t, Fn&& f)
{
    f(t.template get<Idx>(), std::integral_constant<int, Idx>());
}
template <typename T, typename F, int... Is>
void for_each(T&& t, F&& f, std::integer_sequence<int, Is...>)
{
    if constexpr (std::remove_reference_t<T>::Size > 0)
        auto l = {(callWithIdx<Is>(t, f), 0)...};
}
template <int I, int N, typename T1, typename T2, typename F>
void for_each_zipped(T1&& t1, T2&& t2, F&& f)
{
    if constexpr (I != N)
    {
        f(t1.template get<I>(), t2.template get<I>(t2));
        for_each_zipped<I + 1, N, T1, T2, F>(t1, t2, f);
    }
}
} // namespace detail
template <typename... Ts, typename F>
void for_each_in_tuple(fc::tuple<Ts...>& t, F&& f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}
template <int... Is>
constexpr auto reverseIntegerSequence(std::integer_sequence<int, Is...> const&)
{
    return std::integer_sequence<int, sizeof...(Is) - 1 - Is...>{};
}
template <typename... Ts, typename F>
void reverse_for_each_in_tuple(const fc::tuple<Ts...>& t, F&& f)
{
    detail::for_each(t, f,
                     reverseIntegerSequence(std::make_integer_sequence<int, sizeof...(Ts)>()));
}
template <typename... Ts, typename F>
void for_each_in_tuple(const fc::tuple<Ts...>& t, F&& f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}
template <int I, typename T1, typename T2, typename F>
void for_each_zipped(T1& t1, T2& t2, F f)
{
    detail::for_each_zipped<0, I>(t1, t2, f);
}
template <int I, class Fn, class First, class... T>
void for_each_constexpr_impl2(Fn&& fn)
{
    fn(static_cast<First*>(nullptr), std::integral_constant<int, I>());
    if constexpr (sizeof...(T) > 0)
        for_each_constexpr_impl2<I + 1, Fn, T...>(fn);
}
template <class Fn, class... T>
void for_each_constexpr_impl(Fn&& fn, fc::tuple<T...>*)
{
    if constexpr (sizeof...(T) > 0)
        for_each_constexpr_impl2<0, Fn, T...>(fn);
}
template <class Tuple, class Fn>
void for_each_constexpr(Fn&& fn)
{
    for_each_constexpr_impl(fn, static_cast<Tuple*>(nullptr));
}
template <class... Args>
auto make_tuple(Args&&... args)
{
    return fc::tuple<std::remove_reference_t<Args>...>(std::forward<Args>(args)...);
}
} // namespace fc
#endif // FC_FLEXCLASS_TUPLE_HPP
#ifndef FLEXCLASS_UTILITY_HPP
#define FLEXCLASS_UTILITY_HPP
namespace fc
{
//! Utility type to ease SFINAE
template <class T>
struct void_
{
    using type = void;
};
//! Removes all references, const and volatile from T
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
} // namespace fc
#endif
#include <cassert>
#include <new>
#include <type_traits>
namespace fc
{
/*! Base class for a Handle
 *  Defines useful types the library uses
 *  to recognize and operate with the handle
 */
template <class T>
struct Handle
{
    Handle() = default;
    /*! Workaround to allow the handle to be created
     *  from the argument that was passed from the user
     *  The argument (likely the array size) will be
     *  interpreted by the library, which will create
     *  an array and later call "setLocation" on
     *  the handle.
     *  This is why this input is discarded
     */
    template <class U>
    Handle(U&&)
    {
    }
    //! If fc_handle_type is defined, then this is handle
    using fc_handle_type = T;
};
namespace detail
{
//! Placeholder to indicate no iterator was passed by the user
struct NoIterator
{
};
} // namespace detail
/*! internal
 * Class used to store the arguments for an array creation
 * It stores both the array size and an inputiterator that
 * will provide the initial values for the elements
 * If the iterator is the "fc::detail::NoIterator" class, then elements
 * are default initialized
 */
template <class InputIt>
struct Arg
{
    Arg(std::size_t size) : m_size(size) {}
    Arg(std::size_t size, InputIt it) : m_size(size), m_it(it) {}
    std::size_t m_size;
    InputIt m_it;
};
//! Use this as argument for creating an array
inline auto arg(std::size_t size) { return Arg<detail::NoIterator>{size}; }
//! Use this as argument for creating an array with an initial value
//  The input iterator will be called for each element
template <class InputIt>
auto arg(std::size_t size, InputIt it)
{
    return Arg<InputIt>{size, it};
}
template <class InputIt>
auto arg(Arg<InputIt> a)
{
    return a;
}
/*! Placeholder type and values to call ::make to indicate the first
 * argument is an allocator
 */
struct WithAllocator
{
};
static constexpr WithAllocator withAllocator;
/*! internal
 *
 * An ArrayBuilder is responsible for two steps of the process
 * 1 - It is first queried for how many bytes it will need to the
 * array it is supposed to build.
 *
 * 2 - It is provided with a buffer and it should write initialize
 * the array in there. The buffer is guaranteed to fit all data.
 *
 * Exceptions are treated with care. If at any point, an exception
 * is thrown, then all objects already created must be destroyed.
 */
template <class T>
struct ArrayBuilder
{
    //! Destroys all elements in case they are still being tracked
    ~ArrayBuilder()
    {
        if (m_begin)
            reverseDestroy(m_begin, m_end);
    }
    //! Creates the array with inputs specified by Arg in the given buffer.
    template <class InputIt>
    auto buildArray(std::byte* buf, Arg<InputIt>& arg)
    {
        // Find the fist aligned byte suitable for creating T objects
        auto b = aligner(buf).get<T>();
        auto e = b + arg.m_size;
        // In case of an exception, ArrayDeleter will make sure
        //  all objects created up to the point are destroyed
        //  in reverse order
        ArrayDeleter<T> deleter(b);
        for (auto it = b; it != e;)
        {
            // Special handling for default initialization
            if constexpr (std::is_same_v<InputIt, detail::NoIterator>)
                new (it) T;
            else
                new (it) T(*arg.m_it++);
            // Tell the ArrayDeleter another element was createdgc
            deleter.setEnd(++it);
        }
        // No exception was thrown, let the array deleter stop tracking
        // and the ArrayBuilder will track objects from now on
        deleter.release();
        m_begin = b;
        m_end = e;
        return reinterpret_cast<std::byte*>(e);
    }
    //! Query for the number of bytes necessary to create an T array of size
    //! "sz"
    // "offset" is the offset in an imaginary array starting from 0
    template <class InputIt>
    static std::size_t numRequiredBytes(std::size_t offset, const Arg<InputIt>& arg)
    {
        auto numBytes = arg.m_size * sizeof(T);
        auto newOffset = findNextAlignedPosition(offset, alignof(T));
        newOffset += numBytes;
        return newOffset - offset;
    }
    //! Let the array builder stop tracking the array
    void release() { m_begin = m_end = nullptr; }
    T* m_begin{nullptr};
    T* m_end{nullptr};
};
template <class T, class = void>
struct isHandle : std::false_type
{
};
template <class T>
struct isHandle<T, typename void_<typename T::fc_handle_type>::type> : std::true_type
{
    using enable = T;
};
/*! Finds handle types in the types passed by the user
 * and convert them either to "Ignore" or "ArrayBuilder"
 */
template <class T, class = void>
struct ArrayBuildersConverter;
template <class T>
struct ArrayBuildersConverter<T, typename void_<typename isHandle<T>::enable>::type>
{
    using type = ArrayBuilder<typename T::fc_handle_type>;
};
template <class... Args>
auto args(Args&&... args)
{
    return ::fc::make_tuple(fc::arg(std::forward<Args>(args))...);
}
template <class Handles>
struct Handles2ArrayBuilders;
template <class... T>
struct Handles2ArrayBuilders<fc::tuple<T*...>>
{
    using type = fc::tuple<ArrayBuilder<typename T::fc_handle_type>...>;
};
template <class FC, class Alloc, class AArgs, class... ClassArgs>
auto makeWithAllocator(Alloc& alloc, AArgs&& aArgs, ClassArgs&&... cArgs)
{
    using Handles = decltype(std::declval<FC>().fc_handles());
    std::size_t numBytesForArrays = 0;
    for_each_constexpr<Handles>([&](auto* type, auto idx) {
        using Element = remove_cvref_t<decltype(**type)>;
        using Idx = decltype(idx);
        using T = typename Element::fc_handle_type;
        ArrayBuilder<T> arrBuilder;
        numBytesForArrays += arrBuilder.numRequiredBytes(sizeof(FC) + numBytesForArrays,
                                                         aArgs.template get<Idx::value>());
    });
    auto memBuffer = unique_ptr_impl<void, DeleteFn<FC, Alloc>>(
        alloc.allocate(sizeof(FC) + numBytesForArrays), alloc);
    FC* ret;
    if constexpr (std::is_aggregate_v<FC>)
        ret = new (memBuffer.get()) FC{std::forward<ClassArgs>(cArgs)...};
    else
        ret = new (memBuffer.get()) FC(std::forward<ClassArgs>(cArgs)...);
    memBuffer.get_deleter().m_objectCreated = true;
    // Start creating arrays right after the FC object
    std::byte* arrayBuffer = reinterpret_cast<std::byte*>(ret + 1);
    using ArrayBuilders = typename Handles2ArrayBuilders<Handles>::type;
    ArrayBuilders arrayBuilders;
    for_each_in_tuple(arrayBuilders, [&](auto& arrayBuilder, auto idx) mutable {
        using Idx = decltype(idx);
        arrayBuffer = arrayBuilder.buildArray(arrayBuffer, aArgs.template get<Idx::value>());
    });
    auto&& handles = ret->fc_handles();
    for_each_in_tuple(arrayBuilders, [&](auto& arrayBuilder, auto idx) mutable {
        using Idx = decltype(idx);
        handles.template get<Idx::value>()->setLocation(arrayBuilder.m_begin, arrayBuilder.m_end);
        arrayBuilder.release();
    });
    memBuffer.release();
    return ret;
}
template <class FC, class Alloc>
void destroyWithAllocator(Alloc& alloc, FC* p)
{
    if (!p)
        return;
    auto&& handles = p->fc_handles();
    reverse_for_each_in_tuple(handles, [p](auto* handle, auto idx) {
        using Handle = remove_cvref_t<decltype(*handle)>;
        if constexpr (!std::is_trivially_destructible<typename Handle::fc_handle_type>::value)
        {
            reverseDestroy(handle->begin(p), handle->end(p));
        }
    });
    p->~FC();
    alloc.deallocate(const_cast<FC*>(p));
}
template <class FC, class AArgs, class... ClassArgs>
auto makeInternal(AArgs&& aArgs, ClassArgs&&... cArgs)
{
    NewDeleteAllocator alloc;
    return makeWithAllocator<FC>(alloc, std::forward<AArgs>(aArgs),
                                 std::forward<ClassArgs>(cArgs)...);
}
template <class FC>
auto destroy(FC* ptr)
{
    NewDeleteAllocator alloc;
    return destroyWithAllocator<FC>(alloc, ptr);
}
template <class FC, class Alloc>
auto destroy(FC* ptr, Alloc& alloc)
{
    return destroyWithAllocator<FC>(alloc, ptr);
}
template <class FC, class... AArgs>
auto make(AArgs&&... aArgs)
{
    return [a = fc::args(aArgs...)](auto&&... cArgs) mutable {
        return fc::makeInternal<FC>(a, std::forward<decltype(cArgs)>(cArgs)...);
    };
}
template <class FC, class Alloc, class... AArgs>
auto make(WithAllocator, Alloc& alloc, AArgs&&... aArgs)
{
    return [a = fc::args(aArgs...), &alloc](auto&&... cArgs) mutable {
        return fc::makeWithAllocator<FC>(alloc, a, std::forward<decltype(cArgs)>(cArgs)...);
    };
}
template <class T>
struct DestroyFn
{
    void operator()(T* t) { fc::destroy(t); }
};
template <class T, class Deleter = fc::DestroyFn<T>>
using unique_ptr = fc::unique_ptr_impl<T, Deleter>;
template <class FC, class... AArgs>
auto make_unique(AArgs&&... aArgs)
{
    return [a = fc::args(aArgs...)](auto&&... cArgs) mutable {
        return fc::unique_ptr<FC>(fc::makeInternal<FC>(a, std::forward<decltype(cArgs)>(cArgs)...));
    };
}
} // namespace fc
#endif // FC_FLEXCLASS_CORE_HPP
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
    // auto begin() const { return m_begin; }
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
        {
            auto e = ptr->fc_handles().template get<El>()->end(ptr);
            return aligner(e).template get<T>();
        }
    }
};
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
#endif // FC_FLEXCLASS_FLEXCLASS_HPP
